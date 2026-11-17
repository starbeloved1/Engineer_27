#include "engineer_mtc/mtc_node.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <cmath>
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/robot_model/robot_model.h>
#include <moveit/task_constructor/introspection.h>
#include <sstream>
#include <tf2_eigen/tf2_eigen.hpp>

namespace engineer_mtc
{
namespace
{
template <class T> T parameter(rclcpp::Node &node, const std::string &name, const T &fallback)
  {
    return node.has_parameter(name) ? node.get_parameter(name).get_value<T>()
                                    : node.declare_parameter<T>(name, fallback);
  }
} // namespace

MtcNode::MtcNode(const rclcpp::NodeOptions &options) : Node("engineer_mtc", options)
{
  auto text = [this](const char *key, const char *value)
  { return parameter(*this, key, std::string(value)); };
  settings_.group = text("planning_group", "arm_control");
  settings_.tip = text("tip_link", "link_6");
  settings_.base_frame = text("base_frame", "base_link");
  settings_.pipeline = text("planning_pipeline", "ompl");
  settings_.planner_id = text("planner_id", "RRTConnectkConfigDefault");
  settings_.planning_time = parameter(*this, "planning_time", 5.0);
  settings_.velocity_scale = parameter(*this, "velocity_scaling", 0.2);
  settings_.acceleration_scale = parameter(*this, "acceleration_scaling", 0.2);
  settings_.cartesian_step = parameter(*this, "cartesian_step", 0.005);
  settings_.distance = parameter(*this, "pregrasp_distance", 0.07);
  const int max_ik = parameter(*this, "max_ik_solutions", 8);
  if (max_ik < 1 || max_ik > 64)
    throw std::invalid_argument("max_ik_solutions 须在 1 到 64 之间");
  settings_.max_ik_solutions = max_ik;
  settings_.include_approach = parameter(*this, "include_approach", true);
  if (!settings_.include_approach)
    throw std::invalid_argument("完整 pick 要求 include_approach=true");
  max_target_age_ = parameter(*this, "max_target_age", 1.5);
  max_joint_age_ = parameter(*this, "max_joint_age", 1.0);
  target_index_ = parameter(*this, "target_index", 0);
  execution_mode_ = text("execution_mode", "auto");
  if (execution_mode_ != "auto" && execution_mode_ != "step")
    throw std::invalid_argument("execution_mode 须为 auto 或 step");
  const auto zero = std::vector<double>{0, 0, 0};
  object_to_tcp_ = xyzRpyTransform(parameter(*this, "object_to_tcp_xyz", zero),
                                   parameter(*this, "object_to_tcp_rpy", zero));
  settings_.tip_to_tcp = xyzRpyTransform(parameter(*this, "tip_to_tcp_xyz", zero),
                                         parameter(*this, "tip_to_tcp_rpy", zero));
  settings_.object_to_energy = xyzRpyTransform(
      parameter(*this, "object_to_energy_xyz",
                std::vector<double>{.000734246038, -.001457943306, .004333127084}),
      parameter(*this, "object_to_energy_rpy",
                std::vector<double>{3.139041892859, -.000000116949, -.000003732359}));
  parameter(*this, "storage_xyz", std::vector<double>{.25, .2, .35});
  parameter(*this, "storage_rpy", zero);
  auto axis = [this](const char *key, const std::vector<double> &fallback)
  {
    const auto v = parameter(*this, key, fallback);
    if (v.size() != 3)
      throw std::invalid_argument(std::string(key) + " 须有三个分量");
    Eigen::Vector3d a(v[0], v[1], v[2]);
    if (!a.allFinite() || a.norm() < 1e-9)
      throw std::invalid_argument(std::string(key) + " 须为有限非零向量");
    return a.normalized().eval();
  };
  settings_.approach_axis = axis("approach_axis", {1, 0, 0});
  settings_.extraction_axis = axis("extraction_axis", {0, 0, 1});
  settings_.extraction_distance = parameter(*this, "extraction_distance", .10);
  settings_.storage_distance = parameter(*this, "storage_distance", .10);
  settings_.retreat_distance = parameter(*this, "retreat_distance", .07);
  settings_.touch_links = parameter(*this, "touch_links", std::vector<std::string>{"link_6"});
  settings_.gripper_group = text("gripper_group", "gripper");
  settings_.object_id = text("object_id", "energy");
  for (double v :
       {settings_.planning_time, settings_.distance, settings_.cartesian_step, max_target_age_,
        max_joint_age_, settings_.velocity_scale, settings_.acceleration_scale,
        settings_.extraction_distance, settings_.storage_distance, settings_.retreat_distance})
    if (!std::isfinite(v) || v <= 0)
      throw std::invalid_argument("时间、距离及缩放须为有限正数");
  if (settings_.planning_time > 30 || settings_.velocity_scale > 1 ||
      settings_.acceleration_scale > 1 || target_index_ < 0)
    throw std::invalid_argument("规划时间须不超过30s，缩放不超过1，下标非负");
  const auto share = ament_index_cpp::get_package_share_directory("moveit_config");
  energy_ =
      std::make_unique<EnergyScene>(text("energy_urdf", (share + "/energy/energy.urdf").c_str()),
                                    settings_.object_id, settings_.base_frame);
  tf_buffer_ = std::make_unique<tf2_ros::Buffer>(get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
  const auto qos = rclcpp::QoS(1).reliable().transient_local();
  status_pub_ = create_publisher<Status>("~/pick_status", qos);
  stage_pub_ = create_publisher<std_msgs::msg::UInt8>("~/pick_stage", qos);
  gripper_pub_ = create_publisher<std_msgs::msg::Bool>("/gripper_close", qos);
  object_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("~/object_pose_base", qos);
  grasp_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("~/grasp_tcp", qos);
  pregrasp_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("~/pregrasp_pose", qos);
  display_pub_ =
      create_publisher<moveit_msgs::msg::DisplayTrajectory>("~/display_planned_path", qos);
  vision_sub_ = create_subscription<geometry_msgs::msg::PoseArray>(
      text("pose_topic", "/vision/icp/object_poses"), rclcpp::QoS(1).reliable(),
      [this](geometry_msgs::msg::PoseArray::ConstSharedPtr m)
      {
        std::lock_guard<std::mutex> l(input_mutex_);
        vision_ = m;
      });
  manual_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>(
      "~/manual_object_pose", rclcpp::QoS(1).reliable(),
      [this](geometry_msgs::msg::PoseStamped::ConstSharedPtr m)
      {
        std::lock_guard<std::mutex> l(input_mutex_);
        manual_ = m;
      });
  joints_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "/joint_states", rclcpp::SensorDataQoS(),
      std::bind(&MtcNode::updateJoints, this, std::placeholders::_1));
  service_group_ = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  auto service = [this](const std::string &name, auto callback)
  {
    return create_service<Trigger>(name, callback, rmw_qos_profile_services_default,
                                   service_group_);
  };
  plan_service_ = service("~/plan", [this](Trigger::Request::SharedPtr,
                                           Trigger::Response::SharedPtr r) { plan(false, *r); });
  manual_service_ =
      service("~/plan_manual", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { plan(true, *r); });
  execute_service_ =
      service("~/execute", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { startExecution(false, *r); });
  next_service_ =
      service("~/next_stage", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { startExecution(true, *r); });
  cancel_service_ = service("~/cancel", [this](Trigger::Request::SharedPtr,
                                               Trigger::Response::SharedPtr r) { cancel(*r); });
  execute_client_ =
      rclcpp_action::create_client<Execute>(this, "/execute_task_solution", service_group_);
  preview_timer_ =
      create_wall_timer(std::chrono::milliseconds(200), std::bind(&MtcNode::previewTarget, this));
  status_.execution_mode = execution_mode_;
  setStatus(Status::IDLE, 0, "等待有效目标；plan 只规划，execute 执行");
  RCLCPP_INFO(get_logger(),
              "MTC ready: complete pick; mode=%s; plan / execute / next_stage / cancel",
              execution_mode_.c_str());
}

MtcNode::~MtcNode()
{
  cancel_requested_ = true;
  {
    std::lock_guard<std::mutex> l(goal_mutex_);
    if (active_goal_ && rclcpp::ok())
      execute_client_->async_cancel_goal(active_goal_);
  }
  if (worker_.joinable())
    worker_.join();
}

void MtcNode::setStatus(uint8_t phase, uint8_t stage, const std::string &detail)
{
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    status_.phase = phase;
    status_.stage = stage;
    status_.message = detail;
  }
  publishStatus();
  RCLCPP_INFO(get_logger(), "pick stage=%u phase=%u: %s", stage, phase, detail.c_str());
}

void MtcNode::publishStatus()
{
  Status m;
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    status_.header.stamp = now();
    status_.header.frame_id = settings_.base_frame;
    m = status_;
  }
  status_pub_->publish(m);
  std_msgs::msg::UInt8 stage;
  stage.data = m.stage;
  stage_pub_->publish(stage);
}

void MtcNode::updateJoints(sensor_msgs::msg::JointState::ConstSharedPtr m)
{
  {
    std::lock_guard<std::mutex> l(input_mutex_);
    joints_ = m;
  }
  const double age = (now() - rclcpp::Time(m->header.stamp)).seconds();
  if (age < -.1 || age > max_joint_age_)
    return;
  for (size_t i = 0; i < m->name.size() && i < m->position.size(); ++i)
    if (m->name[i] == "gripper_state_joint")
    {
      const double p = m->position[i];
      const bool known = std::isfinite(p) && (std::abs(p) < 1e-4 || std::abs(p - .01) < 1e-4);
      bool changed;
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        const bool closed = std::abs(p) < 1e-4;
        changed = status_.gripper_known != known || (known && status_.gripper_closed != closed);
        status_.gripper_known = known;
        if (known)
          status_.gripper_closed = closed;
      }
      if (changed)
      {
        if (known)
        {
          std_msgs::msg::Bool b;
          b.data = std::abs(p) < 1e-4;
          gripper_pub_->publish(b);
        }
        publishStatus();
      }
    }
}

geometry_msgs::msg::PoseStamped MtcNode::selectTarget(bool manual)
{
  std::lock_guard<std::mutex> l(input_mutex_);
  auto fresh = [this](const auto &stamp, double maximum)
  {
    const double age = (now() - rclcpp::Time(stamp)).seconds();
    return age >= -.1 && age <= maximum;
  };
  if (!joints_ || !fresh(joints_->header.stamp, max_joint_age_))
    throw std::runtime_error("缺少新鲜 /joint_states");
  geometry_msgs::msg::PoseStamped target;
  if (manual)
  {
    if (!manual_)
      throw std::runtime_error("未收到手动目标");
    target = *manual_;
  }
  else
  {
    if (!vision_ || target_index_ >= static_cast<int>(vision_->poses.size()))
      throw std::runtime_error("没有有效视觉目标");
    target.header = vision_->header;
    target.pose = vision_->poses[target_index_];
  }
  if (target.header.frame_id.empty() || !fresh(target.header.stamp, max_target_age_))
    throw std::runtime_error("目标坐标系为空或过期");
  poseTransform(target.pose);
  return target;
}

Eigen::Isometry3d MtcNode::targetInBase(const geometry_msgs::msg::PoseStamped &target)
{
  auto pose = poseTransform(target.pose);
  if (target.header.frame_id != settings_.base_frame)
    pose = tf2::transformToEigen(tf_buffer_->lookupTransform(
               settings_.base_frame, target.header.frame_id, target.header.stamp,
               rclcpp::Duration::from_seconds(.2))) *
           pose;
  return pose;
}

void MtcNode::previewTarget()
{
  bool fresh = false;
  {
    std::lock_guard<std::mutex> l(input_mutex_);
    if (joints_)
    {
      const double age = (now() - rclcpp::Time(joints_->header.stamp)).seconds();
      fresh = age >= -.1 && age <= max_joint_age_;
    }
  }
  bool stale = false;
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    stale = status_.gripper_known && !fresh;
    if (stale)
      status_.gripper_known = false;
  }
  if (stale)
    publishStatus();
  if (busy_ || owned_)
    return;
  std::lock_guard<std::mutex> scene_lock(scene_mutex_);
  if (busy_ || owned_)
    return;
  bool manual = false;
  {
    std::lock_guard<std::mutex> l(input_mutex_);
    manual = manual_ && (!vision_ ||
                         rclcpp::Time(manual_->header.stamp) > rclcpp::Time(vision_->header.stamp));
  }
  bool valid = false;
  try
  {
    const auto target = selectTarget(manual);
    valid = energy_->put(targetInBase(target) * settings_.object_to_energy);
  }
  catch (const std::exception &)
  {
  }
  if (!valid && preview_visible_)
    energy_->remove();
  preview_visible_ = valid;
  bool changed;
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    changed = status_.target_valid != valid;
    status_.target_valid = valid;
  }
  if (changed)
    publishStatus();
}

void MtcNode::publishTargets(const GraspTargets &targets,
                             const builtin_interfaces::msg::Time &stamp)
{
  geometry_msgs::msg::PoseStamped m;
  m.header.frame_id = settings_.base_frame;
  m.header.stamp = stamp;
  m.pose = transformPose(targets.object_base);
  object_pub_->publish(m);
  m.pose = transformPose(targets.grasp_tcp_base);
  grasp_pub_->publish(m);
  m.pose = transformPose(targets.pregrasp_tip_base);
  pregrasp_pub_->publish(m);
}

/*
主链路 plan()
  → selectTarget()          选择目标，检查目标和关节反馈是否有效
  → targetInBase()          把矿位姿转换到 base_link
  → computeGraspTargets()   计算抓取位置
  → EnergyScene::put()      将矿碰撞模型加入场景
  → createPregraspTask()    组装六阶段task
  → task_->plan()           让 MTC 求解完整方案
  → prepareExecution()      将方案整理成六个可执行阶段
*/
void MtcNode::plan(bool manual, Trigger::Response &response)
{
  std::unique_lock<std::mutex> command(command_mutex_);
  if (busy_.exchange(true))
  {
    response.message = "任务忙，本次请求不排队";
    return;
  }
  struct Release
  {
    std::atomic<bool> &flag;
    ~Release()
    {
      flag = false;
    }
  } release{busy_};
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    if (status_.phase == Status::WAITING_NEXT)
    {
      response.message = "调试任务未结束，请继续或取消";
      return;
    }
  }
  plan_valid_ = false;
  next_stage_ = 0;
  cancel_requested_ = false;
  if (worker_.joinable())
    worker_.join();
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    ++status_.task_id;
  }
  setStatus(Status::PLANNING, 0, "规划六阶段完整 pick");
  command.unlock();
  moveit_msgs::msg::DisplayTrajectory empty;
  display_pub_->publish(empty);
  try
  {
    if (energy_->attached())
      throw std::runtime_error("矿仍附着，不能用新目标覆盖；先处理当前持物状态");
    const auto target = selectTarget(manual);
    const auto targets =
        computeGraspTargets(targetInBase(target), object_to_tcp_, settings_.tip_to_tcp,
                            settings_.approach_axis, settings_.distance);
    settings_.storage_energy = xyzRpyTransform(get_parameter("storage_xyz").as_double_array(),
                                               get_parameter("storage_rpy").as_double_array());
    owned_ = true;
    {
      std::lock_guard<std::mutex> l(scene_mutex_);
      if (!energy_->put(targets.object_base * settings_.object_to_energy))
        throw std::runtime_error("添加矿到场景失败");
    }
    {
      std::lock_guard<std::mutex> l(state_mutex_);
      status_.target_valid = true;
      status_.object_attached = false;
      preview_visible_ = true;
    }
    publishTargets(targets, target.header.stamp);
    task_ = createPregraspTask(shared_from_this(), settings_, targets);
    task_->init();
    if (!task_->plan(1) || task_->solutions().empty())
    {
      if (const auto *ik = task_->findChild("stage_2: grasp and attach/grasp IK"))
        for (const auto &candidate : ik->solutions())
        {
          std::vector<double> q;
          candidate->start()->scene()->getCurrentState().copyJointGroupPositions(settings_.group,
                                                                                 q);
          std::ostringstream values;
          for (double x : q)
            values << x << " ";
          RCLCPP_WARN(get_logger(), "grasp IK candidate: %s", values.str().c_str());
        }
      std::ostringstream reason;
      task_->explainFailure(reason);
      throw std::runtime_error("MTC 没有完整六阶段解: " + reason.str());
    }
    // 与 cancel 串行提交规划结果，避免取消请求被刚生成的 READY 状态覆盖。
    std::lock_guard<std::mutex> commit(command_mutex_);
    if (cancel_requested_)
      throw std::runtime_error("规划已取消");
    const auto &solution = *task_->solutions().front();
    task_->introspection().publishSolution(solution);
    prepareExecution();
    moveit_task_constructor_msgs::msg::Solution serialized;
    solution.toMsg(serialized, &task_->introspection());
    moveit_msgs::msg::DisplayTrajectory display;
    display.model_id = task_->getRobotModel()->getName();
    display.trajectory_start = serialized.start_scene.robot_state;
    for (const auto &segment : serialized.sub_trajectory)
      if (!segment.trajectory.joint_trajectory.points.empty())
        display.trajectory.push_back(segment.trajectory);
    if (display.trajectory.empty())
      throw std::runtime_error("完整解没有运动轨迹");
    display_pub_->publish(display);
    plan_valid_ = true;
    response.success = true;
    response.message = "完整 pick 已规划，6 个业务阶段；调用 execute 开始运动";
    setStatus(Status::READY, 0, response.message);
  }
  catch (const std::exception &error)
  {
    owned_ = energy_->attached();
    response.message = error.what();
    setStatus(cancel_requested_ ? Status::CANCELED : Status::FAILED, 0, response.message);
  }
}
} // namespace engineer_mtc
