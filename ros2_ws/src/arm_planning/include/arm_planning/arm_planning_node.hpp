#pragma once

#include <atomic>
#include <rclcpp_action/rclcpp_action.hpp>
#include <thread>

#include "arm_planning/motion_planner.hpp"

namespace arm_planning
{
// 通信与调度层，持有 ROS node 而不与 MoveGroupInterface 形成所有权环
class ArmPlanningNode
{
 public:
  explicit ArmPlanningNode(const rclcpp::Node::SharedPtr& node);
  ~ArmPlanningNode();

 private:
  using Handle = rclcpp_action::ServerGoalHandle<Action>;
  void run(const std::shared_ptr<Handle>& handle);
  void publishFeedback(const std::shared_ptr<Handle>& handle, const std::string& stage,
                       const std::string& detail);
  static void checkCanceled(const std::shared_ptr<Handle>& handle);

  rclcpp::Node::SharedPtr node_;
  PlannerSettings settings_;
  MotionPlanner planner_;
  rclcpp_action::Server<Action>::SharedPtr server_;
  rclcpp::TimerBase::SharedPtr cancel_timer_;
  std::atomic<bool> busy_{false};
  std::atomic<bool> cancel_requested_{false};
  std::thread worker_;
};
}  // namespace arm_planning
