#include "engineer_mtc/mtc_node.hpp"
#include <cmath>
#include <moveit/robot_model/robot_model.h>
#include <moveit/task_constructor/introspection.h>
#include <moveit_task_constructor_msgs/msg/solution.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

namespace engineer_mtc
{
namespace
{
template <class T>
T parameter(rclcpp::Node& node, const std::string& name, const T& fallback)
{
  return node.has_parameter(name) ? node.get_parameter(name).get_value<T>() :
                                    node.declare_parameter<T>(name, fallback);
}
}  // namespace

MtcNode::MtcNode(const rclcpp::NodeOptions& options) : Node("engineer_mtc", options)
{
  auto text = [this](const char* key, const char* value) {
    return parameter(*this, key, std::string(value));
  };
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
  const int max_ik_solutions = parameter(*this, "max_ik_solutions", 8);
  if (max_ik_solutions < 1 || max_ik_solutions > 64)
    throw std::invalid_argument("max_ik_solutions 须在 1 到 64 之间");
  settings_.max_ik_solutions = static_cast<unsigned int>(max_ik_solutions);
  settings_.include_approach = parameter(*this, "include_approach", false);
  max_target_age_ = parameter(*this, "max_target_age", 1.5);
  max_joint_age_ = parameter(*this, "max_joint_age", 1.0);
  target_index_ = parameter(*this, "target_index", 0);
  const auto zero = std::vector<double>{0.0, 0.0, 0.0};
  object_to_tcp_ = xyzRpyTransform(parameter(*this, "object_to_tcp_xyz", zero),
                                  parameter(*this, "object_to_tcp_rpy", zero));
  settings_.tip_to_tcp = xyzRpyTransform(parameter(*this, "tip_to_tcp_xyz", zero),
                                        parameter(*this, "tip_to_tcp_rpy", zero));
  const auto axis = parameter(*this, "approach_axis", std::vector<double>{1.0, 0.0, 0.0});
  if (axis.size() != 3) throw std::invalid_argument("approach_axis 须包含三个数");
  settings_.approach_axis = Eigen::Vector3d(axis[0], axis[1], axis[2]);
  for (double v : {settings_.planning_time, settings_.distance, settings_.cartesian_step,
                   max_target_age_, max_joint_age_, settings_.velocity_scale,
                   settings_.acceleration_scale})
    if (!std::isfinite(v) || v <= 0) throw std::invalid_argument("时间、距离及缩放须为有限正数");
  if (settings_.planning_time > 30 || settings_.velocity_scale > 1 ||
      settings_.acceleration_scale > 1 || target_index_ < 0)
    throw std::invalid_argument("规划时间须不超过 30s，缩放须不超过 1，目标下标须非负");
  computeGraspTargets(Eigen::Isometry3d::Identity(), object_to_tcp_, settings_.tip_to_tcp,
                       settings_.approach_axis, settings_.distance);

  vision_sub_ = create_subscription<geometry_msgs::msg::PoseArray>(
      text("pose_topic", "/vision/icp/object_poses"), rclcpp::QoS(1).reliable(),
      [this](geometry_msgs::msg::PoseArray::ConstSharedPtr message) {
        std::lock_guard<std::mutex> lock(input_mutex_);
        vision_ = message;  // 空数组也替换旧结果，不能无限沿用失效目标。
      });

  manual_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>(
      "~/manual_object_pose", rclcpp::QoS(1).reliable(),
      [this](geometry_msgs::msg::PoseStamped::ConstSharedPtr message) {
        std::lock_guard<std::mutex> lock(input_mutex_);
        manual_ = message;
      });

  joints_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "/joint_states", rclcpp::SensorDataQoS(),
      [this](sensor_msgs::msg::JointState::ConstSharedPtr message) {
        std::lock_guard<std::mutex> lock(input_mutex_);
        joints_ = message;
      });

  tf_buffer_ = std::make_unique<tf2_ros::Buffer>(get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
  const auto output_qos = rclcpp::QoS(1).reliable().transient_local();
  object_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("~/object_pose_base", output_qos);
  grasp_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("~/grasp_tcp", output_qos);
  pregrasp_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("~/pregrasp_pose", output_qos);
  display_pub_ = create_publisher<moveit_msgs::msg::DisplayTrajectory>(
      "~/display_planned_path", output_qos);
  service_group_ = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  plan_service_ = create_service<Trigger>(
      "~/plan", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr response) {
        plan(false, *response);
      }, rmw_qos_profile_services_default, service_group_);
  manual_service_ = create_service<Trigger>(
      "~/plan_manual", [this](Trigger::Request::SharedPtr, Trigger::Response::SharedPtr response) {
        plan(true, *response);
      }, rmw_qos_profile_services_default, service_group_);
  RCLCPP_INFO(get_logger(), "MTC ready: /engineer_mtc/plan (vision), /engineer_mtc/plan_manual; "
              "group=%s, tip=%s, approach=%d; planning only", settings_.group.c_str(),
              settings_.tip.c_str(), settings_.include_approach);
}

// 检查输入
geometry_msgs::msg::PoseStamped MtcNode::selectTarget(bool manual)
{
  geometry_msgs::msg::PoseStamped target;
  std::lock_guard<std::mutex> lock(input_mutex_);
  auto fresh = [this](const auto& stamp, double maximum) {
    const double age = (now() - rclcpp::Time(stamp, get_clock()->get_clock_type())).seconds();
    return age >= -0.1 && age <= maximum;
  };
  if (!joints_ || !fresh(joints_->header.stamp, max_joint_age_))
    throw std::runtime_error("没有新鲜的 /joint_states，请先启动模拟控制或实际关节反馈");
  if (manual)
  {
    if (!manual_) throw std::runtime_error("尚未收到 manual_object_pose");
    target = *manual_;
  }
  else
  {
    if (!vision_ || target_index_ >= static_cast<int>(vision_->poses.size()))
      throw std::runtime_error("没有当前有效视觉目标，或 target_index 超出数组范围");
    target.header = vision_->header;
    target.pose = vision_->poses[target_index_];
  }
  if (target.header.frame_id.empty() || !fresh(target.header.stamp, max_target_age_))
    throw std::runtime_error("目标坐标系为空或目标已过期，请获取新观测");
  poseTransform(target.pose);
  return target;
}

void MtcNode::publishTargets(const GraspTargets& targets,
                             const builtin_interfaces::msg::Time& stamp)
{
  geometry_msgs::msg::PoseStamped message;
  message.header.frame_id = settings_.base_frame;
  message.header.stamp = stamp;
  message.pose = transformPose(targets.object_base);
  object_pub_->publish(message);
  message.pose = transformPose(targets.grasp_tcp_base);
  grasp_pub_->publish(message);
  message.pose = transformPose(targets.pregrasp_tip_base);
  pregrasp_pub_->publish(message);
}

/*
core:
1.selectTargets()
2.转换到baselink，调用computeGraspTargets()构造GraspTargets发布到对应topic
3.createPregraspTask()构造Mtc任务
4.提取运动并发布轨迹预览（rviz）
*/
void MtcNode::plan(bool manual, Trigger::Response& response)
{
  if (busy_.exchange(true))
  {
    response.message = "MTC 正在规划，本次请求不排队";
    return;
  }
  struct Release { std::atomic<bool>& flag; ~Release() { flag = false; } } release{busy_};
  try
  {
    // 一次请求冻结一份带时间戳的目标，后续视觉消息不会改变正在计算的任务。
    const auto target = selectTarget(manual);
    auto object_base = poseTransform(target.pose);
    if (target.header.frame_id != settings_.base_frame)
    {
      const auto tf = tf_buffer_->lookupTransform(settings_.base_frame, target.header.frame_id,
                                                  target.header.stamp,
                                                  rclcpp::Duration::from_seconds(0.2));
      object_base = tf2::transformToEigen(tf) * object_base;
    }
    const auto targets = computeGraspTargets(object_base, object_to_tcp_, settings_.tip_to_tcp,
                                             settings_.approach_axis, settings_.distance);
    publishTargets(targets, target.header.stamp);
    RCLCPP_INFO(get_logger(), "object -> TCP -> %s pregrasp: [%.4f, %.4f, %.4f] m",
                settings_.tip.c_str(), targets.pregrasp_tip_base.translation().x(),
                targets.pregrasp_tip_base.translation().y(),
                targets.pregrasp_tip_base.translation().z());
    task_ = createPregraspTask(shared_from_this(), settings_, targets);
    task_->init();
    if (!task_->plan(1) || task_->solutions().empty())
      throw std::runtime_error("MTC 未找到完整任务解，请查看各阶段失败日志");
    const auto& solution = *task_->solutions().front();
    task_->introspection().publishSolution(solution);
    moveit_task_constructor_msgs::msg::Solution serialized;
    solution.toMsg(serialized, &task_->introspection());
    moveit_msgs::msg::DisplayTrajectory display;
    display.model_id = task_->getRobotModel()->getName();
    display.trajectory_start = serialized.start_scene.robot_state;
    for (const auto& segment : serialized.sub_trajectory)
      if (!segment.trajectory.joint_trajectory.points.empty())
        display.trajectory.push_back(segment.trajectory);
    if (display.trajectory.empty()) throw std::runtime_error("MTC 返回的运动轨迹为空");
    display_pub_->publish(display);
    response.success = true;
    response.message = "MTC 规划成功，共 " + std::to_string(display.trajectory.size()) +
                       " 段运动；已发布 RViz 预览，未执行运动";
    RCLCPP_INFO(get_logger(), "%s", response.message.c_str());
  }
  catch (const std::exception& error)
  {
    response.message = error.what();
    RCLCPP_WARN(get_logger(), "MTC planning failed: %s", error.what());
  }
}
}  // namespace engineer_mtc
