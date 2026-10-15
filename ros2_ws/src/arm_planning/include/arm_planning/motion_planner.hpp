#pragma once

#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/robot_trajectory/robot_trajectory.h>

#include <act_interfaces/action/plan_motion.hpp>
#include <functional>
#include <memory>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <moveit_msgs/srv/get_planning_scene.hpp>
#include <rclcpp/rclcpp.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace arm_planning
{
using Action = act_interfaces::action::PlanMotion;
using Codes = moveit_msgs::msg::MoveItErrorCodes;
using State = moveit::core::RobotState;
using Scene = planning_scene::PlanningScene;

struct PlanningError : std::runtime_error
{
  int code;
  PlanningError(int value, const std::string& message) : std::runtime_error(message), code(value) {}
};

// 参数由 arm_planning.yaml 提供，算法层不创建 Action 服务端
struct PlannerSettings
{
  explicit PlannerSettings(const rclcpp::Node::SharedPtr& node);
  std::string planning_action, scene_service, display_topic;
  std::string planning_group, end_effector_link, planning_pipeline, planner_id;
  std::vector<std::string> reference_frames;
  double validation_step, ik_timeout, joint_tolerance;
  double default_velocity, default_acceleration, default_planning_time;
  int ik_attempts;
};

class MotionPlanner
{
 public:
  using Feedback = std::function<void(const std::string&, const std::string&)>;
  using CheckCanceled = std::function<void()>;
  MotionPlanner(const rclcpp::Node::SharedPtr& node, const PlannerSettings& settings);
  bool validGoal(const Action::Goal& goal) const;
  void plan(const Action::Goal& goal, Action::Result& result, const Feedback& feedback,
            const CheckCanceled& check_canceled);
  void stop();

 private:
  std::shared_ptr<Scene> snapshot();
  std::string collision(const std::shared_ptr<Scene>& scene, const State& state);
  std::vector<double> positions(const State& state);
  State nearStart(const State& target, const State& start);
  State poseTarget(const Action::Goal& goal, const CheckCanceled& check_canceled,
                   const State& start, const std::shared_ptr<Scene>& scene, Action::Result& result);
  uint32_t validatePath(const CheckCanceled& check_canceled, const std::shared_ptr<Scene>& scene,
                        const robot_trajectory::RobotTrajectory& trajectory);

  PlannerSettings settings_;
  std::unique_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;
  const moveit::core::JointModelGroup* group_{};
  rclcpp::Client<moveit_msgs::srv::GetPlanningScene>::SharedPtr scene_client_;
  rclcpp::Publisher<moveit_msgs::msg::DisplayTrajectory>::SharedPtr display_;
};
}  // namespace arm_planning
