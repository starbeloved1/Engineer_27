#include "engineer_mtc/pick/task.hpp"
#include "engineer_mtc/place/task.hpp"
#include "engineer_mtc/node/mtc_node.hpp"
#include "engineer_mtc/shared/motion_stages.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <cmath>
#include <limits>
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/robot_model/robot_model.h>
#include <moveit/task_constructor/storage.h>
#include <optional>
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
  const auto maxIkSolutions = [this](const std::string &name, int default_value) {
    const int count = parameter(*this, name, default_value);
    if (count < 1 || count > 64)
      throw std::invalid_argument(name + " 须在 1 到 64 之间");
    return static_cast<unsigned int>(count);
  };
  settings_.max_ik_solutions = maxIkSolutions("max_ik_solutions", 10);
  settings_.pick_max_ik_solutions = maxIkSolutions("pick_max_ik_solutions", 20);
  settings_.include_approach = parameter(*this, "include_approach", true);
  if (!settings_.include_approach)
    throw std::invalid_argument("外部取矿要求 include_approach=true");
  max_target_age_ = parameter(*this, "max_target_age", 1.5);
  max_joint_age_ = parameter(*this, "max_joint_age", 1.0);
  hardware_timeout_ = parameter(*this, "hardware_status_timeout", .1);
  target_index_ = parameter(*this, "target_index", 0);
  target_average_samples_ = parameter(*this, "target_average_samples", 5);
  target_average_max_age_ = parameter(*this, "target_average_max_age", 5.0);
  if (target_average_samples_ < 1 || target_average_samples_ > 100)
    throw std::invalid_argument("target_average_samples 须为 1 到 100 的整数");
  execution_mode_ = text("execution_mode", "auto");
  if (execution_mode_ != "auto" && execution_mode_ != "step")
    throw std::invalid_argument("execution_mode 须为 auto 或 step");
  const auto zero = std::vector<double>{0, 0, 0};
  object_to_tcp_ = xyzRpyTransform(
      parameter(*this, "object_to_tcp_xyz",
                std::vector<double>{-.000734251986318, -.001468988580768, .004329394048142}),
      parameter(*this, "object_to_tcp_rpy",
                std::vector<double>{.002550760731211, -.000000107428275, -.000003732645167}));
  settings_.tip_to_tcp = xyzRpyTransform(parameter(*this, "tip_to_tcp_xyz", zero),
                                         parameter(*this, "tip_to_tcp_rpy", zero));
  parameter(*this, "storage_xyz", std::vector<double>{-.277770576390778, .230947485257963, .19});
  parameter(*this, "storage_rpy", std::vector<double>{3.141592653589793, 0., 2.09439510});
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
  settings_.extraction_axis = axis("extraction_axis",
                                 {.000000116949, .002550757964758, -.999996746811604});
  settings_.storage_axis = axis("storage_insertion_axis", {0, 0, -1});
  settings_.storage_contact_objects = parameter(*this, "storage_contact_objects", std::vector<std::string>{});
  settings_.extraction_distance = parameter(*this, "extraction_distance", .10);
  settings_.storage_distance = parameter(*this, "storage_distance", .10);
  settings_.retreat_distance = parameter(*this, "retreat_distance", .07);
  settings_.touch_links = parameter(*this, "touch_links", std::vector<std::string>{"link_6"});
  settings_.gripper_group = text("gripper_group", "gripper");
  settings_.object_id = text("object_id", "energy");
  for (double v :
       {settings_.planning_time, settings_.distance, settings_.cartesian_step, max_target_age_,
        max_joint_age_, target_average_max_age_, hardware_timeout_, settings_.velocity_scale, settings_.acceleration_scale,
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
  preview_pub_ = create_publisher<visualization_msgs::msg::MarkerArray>("~/preview_markers", qos);
  clearPreview();
  vision_sub_ = create_subscription<geometry_msgs::msg::PoseArray>(
      text("pose_topic", "/vision/icp/object_poses"), rclcpp::QoS(1).reliable(),
      [this](geometry_msgs::msg::PoseArray::ConstSharedPtr m)
      {
        std::lock_guard<std::mutex> l(input_mutex_);
        vision_ = m;
        vision_history_.push(*m, static_cast<size_t>(target_index_),
                             static_cast<size_t>(target_average_samples_), max_target_age_);
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
  hardware_sub_ = create_subscription<serial_driver::msg::HardwareStatus>(
      "/serial_driver/status", qos,
      std::bind(&MtcNode::updateHardware, this, std::placeholders::_1));
  service_group_ = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  auto service = [this](const std::string &name, auto callback)
  {
    return create_service<Trigger>(name, callback, rmw_qos_profile_services_default,
                                   service_group_);
  };
  plan_service_ = service("~/plan", [this](Trigger::Request::SharedPtr,
                                           Trigger::Response::SharedPtr r) { plan(PlanRequest::Vision, *r); });
  manual_service_ =
      service("~/plan_manual", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { plan(PlanRequest::Manual, *r); });
  place_service_ =
      service("~/plan_place", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { plan(PlanRequest::Place, *r); });
  execute_service_ =
      service("~/execute", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { startExecution(false, *r); });
  next_service_ =
      service("~/next_stage", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
              { startExecution(true, *r); });
  cancel_service_ = service("~/cancel", [this](Trigger::Request::SharedPtr,
                                               Trigger::Response::SharedPtr r) { cancel(*r); });
  replay_service_ = service("~/replay", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr r)
  {
    std::lock_guard<std::mutex> lock(command_mutex_);
    if (busy_ || replay_trajectory_.trajectory.empty())
    {
      r->message = "当前没有可重播的完整规划";
      return;
    }
    display_pub_->publish(replay_trajectory_);
    r->success = true;
    r->message = "已重播完整规划（仅动画预览）";
  });
  execute_client_ =
      rclcpp_action::create_client<Execute>(this, "/execute_task_solution", service_group_);
  preview_timer_ =
      create_wall_timer(std::chrono::milliseconds(200), std::bind(&MtcNode::previewTarget, this));
  status_.execution_mode = execution_mode_;
  setStatus(Status::IDLE, 0, "等待有效目标；plan 只规划，execute 执行");
  RCLCPP_INFO(get_logger(),
              "MTC ready: mode=%s; plan / plan_place / execute / next_stage / cancel / replay",
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
  if (phase == Status::CANCELED)
    clearPreview();
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
  bool gripper_confirmed = false;
  {
    std::lock_guard<std::mutex> l(input_mutex_);
    joints_ = m;
    gripper_confirmed = hardware_ && hardware_->feedback_valid && !hardware_->faulted &&
        (hardware_->gripper_state & 7) == 1 &&
        std::abs((now() - rclcpp::Time(hardware_->header.stamp)).seconds()) <= hardware_timeout_;
  }
  const double age = (now() - rclcpp::Time(m->header.stamp)).seconds();
  if (age < -.1 || age > max_joint_age_)
    return;
  for (size_t i = 0; i < m->name.size() && i < m->position.size(); ++i)
    if (m->name[i] == "gripper_state_joint")
    {
      const double p = m->position[i];
      const bool known = gripper_confirmed && std::isfinite(p) &&
                         (std::abs(p) < 1e-4 || std::abs(p - .01) < 1e-4);
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

// SelectTarget() 选择目标
geometry_msgs::msg::PoseStamped MtcNode::selectTarget(bool manual)
{
  geometry_msgs::msg::PoseStamped target;
  std::vector<geometry_msgs::msg::PoseStamped> samples;
  {
    std::lock_guard<std::mutex> l(input_mutex_);
    auto fresh = [this](const auto &stamp, double maximum)
    {
      const double age = (now() - rclcpp::Time(stamp)).seconds();
      return age >= -.1 && age <= maximum;
    };
    if (!joints_ || !fresh(joints_->header.stamp, max_joint_age_))
      throw std::runtime_error("缺少新鲜 /joint_states");
    if (manual)
    {
      if (!manual_) throw std::runtime_error("未收到手动目标");
      target = *manual_;
      if (target.header.frame_id.empty() || !fresh(target.header.stamp, max_target_age_))
        throw std::runtime_error("目标坐标系为空或过期");
      poseTransform(target.pose);
      return target;
    }
    if (!vision_ || target_index_ >= static_cast<int>(vision_->poses.size()))
      throw std::runtime_error("没有有效视觉目标");
    samples = vision_history_.recent(now().seconds(), target_average_max_age_,
                                     static_cast<size_t>(target_average_samples_), max_target_age_);
  }
  // 每帧先按自己的采集时刻变换到 base，避免相机 TF 的变化被混入均值。
  std::vector<geometry_msgs::msg::Pose> poses;
  poses.reserve(samples.size());
  for (const auto &sample : samples) poses.push_back(transformPose(targetInBase(sample)));
  target.header = samples.front().header;  // 保留最旧样本时间戳，保守判断均值的新鲜度。
  target.header.frame_id = settings_.base_frame;
  target.pose = averagePoses(poses);
  if ((now() - rclcpp::Time(samples.back().header.stamp)).seconds() > max_target_age_ ||
      (now() - rclcpp::Time(target.header.stamp)).seconds() > target_average_max_age_)
    throw std::runtime_error("视觉取样在坐标变换期间已过期，请重试");
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
  // 规划和 READY 预览不检查执行使能，模拟控制器未激活时仍可规划。
  if (executionNeedsHardware() && !cancel_requested_)
  {
    try { checkHardware(); }
    catch (const std::exception &e)
    {
      RCLCPP_WARN(get_logger(), "硬件监测取消任务: %s", e.what());
      Trigger::Response response;
      cancel(response);
    }
  }
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
    const auto pose = targetInBase(target);
    const auto now_steady = std::chrono::steady_clock::now();
    if (preview_visible_ && now_steady - last_preview_update_ < std::chrono::seconds(1))
      valid = true;
    else
    {
      valid = energy_->put(pose);
      if (valid) last_preview_update_ = now_steady;
    }
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

void MtcNode::publishTargets(const GraspTargets &targets,const builtin_interfaces::msg::Time &stamp)
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

void MtcNode::checkFreshJoints()
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  if (!joints_)
    throw std::runtime_error("缺少新鲜 /joint_states");
  const double age = (now() - rclcpp::Time(joints_->header.stamp)).seconds();
  if (age < -.1 || age > max_joint_age_)
    throw std::runtime_error("缺少新鲜 /joint_states");
}

/*
Plan() 主入口
1.selectTarget() 拿到本次使用的矿位姿
2.统一到基座坐标系，computeGraspTargets() 拿到打包好的 GraspTargets
3.将矿加入规划场景，使用 energy_->put()
4.发布位姿和可视化相关
5.createPregraspTask()组装 stage_1，task_->plan() 生成预抓取候选
6.createPickTailTask()组装 stage_2&3，tail->plan() 生成候选
7.选择两段总cost最小的完整解，保存并发布结果
*/
void MtcNode::plan(PlanRequest request, Trigger::Response &response)
{
  // 检查
  const bool place = request == PlanRequest::Place;
  const std::string task_name = place ? "持矿存车" : "外部取矿";
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
  if (execution_unconfirmed_)
  {
    response.message = "旧轨迹是否停止或碰撞许可是否恢复尚未确认，请检查控制器和规划场景";
    return;
  }
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    if (status_.phase == Status::WAITING_NEXT)
    {
      response.message = "调试任务未结束，请继续或取消";
      return;
    }
  }
  if (place && !extracted_)
  {
    response.message = "存车需要已完成拔离的矿";
    return;
  }
  plan_valid_ = false;
  replay_trajectory_ = moveit_msgs::msg::DisplayTrajectory{};
  next_stage_ = place ? 3 : 0;
  end_stage_ = place ? 6 : 3;
  cancel_requested_ = false;
  if (worker_.joinable())
    worker_.join();
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    ++status_.task_id;
  }

  // 进入planning状态
  setStatus(Status::PLANNING, 0, "规划" + task_name);
  command.unlock();
  moveit_msgs::msg::DisplayTrajectory empty;
  display_pub_->publish(empty);
  clearPreview();

  try
  {
    pick_tail_task_.reset();
    std::optional<GraspTargets> pick_targets;
    if (place) // place 分支
    {
      checkFreshJoints();
      energy_->checkStorage(storedObjectId(settings_.object_id), settings_.storage_contact_objects);
      const auto attached = energy_->attachedPose(settings_.tip);
      baseline_acm_ = energy_->collisionMatrix();
      const auto storage_energy = xyzRpyTransform(get_parameter("storage_xyz").as_double_array(),
                                                  get_parameter("storage_rpy").as_double_array());
      const Eigen::Isometry3d storage_tip = storage_energy * attached.inverse();
      const Eigen::Isometry3d storage_tcp = storage_tip * settings_.tip_to_tcp;
      Eigen::Isometry3d prestorage_tcp = storage_tcp;
      prestorage_tcp.translation() -= settings_.storage_axis * settings_.storage_distance;
      publishPreview(prestorage_tcp, storage_tcp, true);
      task_ = createPlaceTask(shared_from_this(), settings_, storage_tip, CollisionMatrix(baseline_acm_));
    }
    else // pick 分支
    {
      if (energy_->attached())
        throw std::runtime_error("矿仍附着，不能用新目标覆盖；先处理当前持物状态");
      if (contact_state_needs_review_)
        throw std::runtime_error("上次接触动作中断，请先确认矿和场景状态");
      extracted_ = false;
      const auto target = selectTarget(request == PlanRequest::Manual);
      pick_targets = computeGraspTargets(targetInBase(target), object_to_tcp_, settings_.tip_to_tcp,settings_.approach_axis, settings_.distance);
      const auto &targets = *pick_targets;
      owned_ = true;
      {
        std::lock_guard<std::mutex> l(scene_mutex_);
        if (!energy_->put(targets.object_base))
          throw std::runtime_error("添加矿到场景失败");
      }
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        status_.target_valid = true;
        status_.object_attached = false;
        preview_visible_ = true;
      }
      publishTargets(targets, target.header.stamp);
      publishPreview(targets.pregrasp_tip_base * settings_.tip_to_tcp, targets.grasp_tcp_base, false);
      task_ = createPregraspTask(shared_from_this(), settings_, targets);
    }
    task_->init();
    const auto max_task_solutions = place ? 8 : 20;
    const auto solve = [this](moveit::task_constructor::Task &task, size_t count)
    {
      {
        std::lock_guard<std::mutex> lock(planning_mutex_);
        if (cancel_requested_) throw std::runtime_error("规划已取消");
        active_planning_task_ = &task;
      }
      struct ClearActive
      {
        MtcNode *node;
        ~ClearActive()
        {
          std::lock_guard<std::mutex> lock(node->planning_mutex_);
          node->active_planning_task_ = nullptr;
        }
      } clear{this};
      const auto result = task.plan(count);
      if (cancel_requested_) throw std::runtime_error("规划已取消");
      return result;
    };
    setStatus(Status::PLANNING, 0, task_name + "：正在规划主任务");
    RCLCPP_INFO(get_logger(), "%s：开始主任务规划", task_name.c_str());
    if (!solve(*task_, max_task_solutions) || task_->solutions().empty())
    {
      if (const auto *ik = task_->findChild("stage_1: pregrasp/pregrasp IK"))
        for (const auto &candidate : ik->solutions())
        {
          std::vector<double> q;
          candidate->start()->scene()->getCurrentState().copyJointGroupPositions(settings_.group,
                                                                                 q);
          std::ostringstream values;
          for (double x : q)
            values << x << " ";
          RCLCPP_WARN(get_logger(), "pregrasp IK candidate: %s", values.str().c_str());
        }
      std::ostringstream reason;
      task_->explainFailure(reason);
      throw std::runtime_error(task_name + "规划失败: " + reason.str());
    }
    std::unique_ptr<moveit::task_constructor::Task> selected_tail_task;
    RCLCPP_INFO(get_logger(), "%s：主任务规划完成，%zu 个候选", task_name.c_str(), task_->solutions().size());
    moveit_task_constructor_msgs::msg::Solution serialized, serialized_tail;
    moveit::task_constructor::SolutionBaseConstPtr selected, selected_tail;
    double selected_cost = std::numeric_limits<double>::infinity();
    size_t complete_pick_candidates = 0;
    std::string rejected;
    const auto serializeExecutable = [&](const moveit::task_constructor::SolutionBase &solution,
                                         moveit_task_constructor_msgs::msg::Solution &message)
    {
      solution.toMsg(message);
      try
      {
        checkTrajectoryLimits(message);
      }
      catch (const std::runtime_error &error)
      {
        rejected = error.what();
        return false;
      }
      return true;
    };
    size_t candidate_index = 0;
    for (const auto &candidate : task_->solutions())
    {
      ++candidate_index;
      RCLCPP_INFO(get_logger(), "候选 %zu/%zu：开始序列化和轨迹检查", candidate_index, task_->solutions().size());
      if (cancel_requested_)
        throw std::runtime_error("规划已取消");
      moveit_task_constructor_msgs::msg::Solution candidate_message;
      if (!serializeExecutable(*candidate, candidate_message))
        continue;
      if (place)
      {
        serialized = std::move(candidate_message);
        selected = candidate;
        selected_cost = candidate->cost();
        break;
      }

      // 为每个预抓取末态独立规划抓取和拉出，按两份轨迹的总代价选解
      setStatus(Status::PLANNING, 0, "抓取拔出候选 " + std::to_string(candidate_index) + "/" + std::to_string(task_->solutions().size()));
      RCLCPP_INFO(get_logger(), "候选 %zu：创建抓取和拔出任务", candidate_index);
      auto tail = createPickTailTask(shared_from_this(), settings_, *pick_targets, candidate->end()->scene());
      tail->init();
      RCLCPP_INFO(get_logger(), "候选 %zu：开始抓取和拔出规划", candidate_index);
      if (!solve(*tail, max_task_solutions) || tail->solutions().empty())
      {
        std::ostringstream reason;
        tail->explainFailure(reason);
        rejected = "接近夹紧或拉出规划失败: " + reason.str();
        continue;
      }
      RCLCPP_INFO(get_logger(), "候选 %zu：抓取和拔出规划完成，%zu 个解", candidate_index, tail->solutions().size());
      for (const auto &tail_candidate : tail->solutions())
      {
        moveit_task_constructor_msgs::msg::Solution tail_message;
        RCLCPP_INFO(get_logger(), "候选 %zu：开始抓取和拔出轨迹检查", candidate_index);
        if (!serializeExecutable(*tail_candidate, tail_message))
          continue;
        ++complete_pick_candidates;
        const double cost = candidate->cost() + tail_candidate->cost();
        if (cost < selected_cost)
        {
          serialized = std::move(candidate_message);
          serialized_tail = std::move(tail_message);
          selected = candidate;
          selected_tail = tail_candidate;
          RCLCPP_INFO(get_logger(), "候选 %zu：更新最优解，释放旧候选任务", candidate_index);
          selected_tail_task = std::move(tail);
          RCLCPP_INFO(get_logger(), "候选 %zu：最优解更新完成", candidate_index);
          selected_cost = cost;
        }
        break; 
      }
      RCLCPP_INFO(get_logger(), "候选 %zu：开始清理未选中的任务", candidate_index);
      tail.reset();
      RCLCPP_INFO(get_logger(), "候选 %zu：清理完成", candidate_index);
    }

    RCLCPP_INFO(get_logger(), "%s：候选处理完成，开始提交规划结果", task_name.c_str());
    std::lock_guard<std::mutex> commit(command_mutex_);
    if (cancel_requested_)
      throw std::runtime_error("规划已取消");
    if (!selected)
      throw std::runtime_error(task_name + "候选解均不可执行：" + rejected);
    RCLCPP_INFO(get_logger(), "%s：找到 %zu 个完整解，选中代价 %.3f",
                task_name.c_str(), place ? task_->solutions().size() : complete_pick_candidates, selected_cost);
    stage_solutions_.fill(moveit_task_constructor_msgs::msg::Solution{});
    prepareExecution(*task_, *selected, next_stage_, place ? end_stage_ : 1);
    if (selected_tail)
      prepareExecution(*selected_tail_task, *selected_tail, 1, end_stage_);
    moveit_msgs::msg::DisplayTrajectory display;
    display.model_id = task_->getRobotModel()->getName();
    display.trajectory_start = serialized.start_scene.robot_state;
    for (const auto *solution : {&serialized, &serialized_tail})
      for (const auto &segment : solution->sub_trajectory)
        if (!segment.trajectory.joint_trajectory.points.empty())
          display.trajectory.push_back(segment.trajectory);
    if (display.trajectory.empty())
      throw std::runtime_error("完整解没有运动轨迹");
    if (selected_tail)
    {
      pick_tail_task_ = std::move(selected_tail_task);
    }
    replay_trajectory_ = display;
    display_pub_->publish(display);
    plan_valid_ = true;
    response.success = true;
    response.message = task_name + "已规划，调用 execute 开始运动";
    setStatus(Status::READY, 0, response.message);
  }
  catch (const std::exception &error)
  {
    owned_ = contact_state_needs_review_ || energy_->attached();
    response.message = error.what();
    setStatus(cancel_requested_ ? Status::CANCELED : Status::FAILED, 0, response.message);
  }
}
} // namespace engineer_mtc
