#pragma once
#include "engineer_mtc/energy_scene.hpp"
#include "engineer_mtc/tasks.hpp"
#include "engineer_mtc/vision_target_buffer.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <engineer_mtc/msg/pick_status.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <moveit_task_constructor_msgs/action/execute_task_solution.hpp>
#include <mutex>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <serial_driver/msg/hardware_status.hpp>
#include <std_msgs/msg/bool.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <thread>

namespace engineer_mtc
{
class MtcNode : public rclcpp::Node
{
public:
  explicit MtcNode(const rclcpp::NodeOptions &options);
  ~MtcNode() override;

private:
  using Trigger = std_srvs::srv::Trigger;
  using Status = msg::PickStatus;
  using Execute = moveit_task_constructor_msgs::action::ExecuteTaskSolution;
  using GoalHandle = rclcpp_action::ClientGoalHandle<Execute>;
  geometry_msgs::msg::PoseStamped selectTarget(bool manual);
  Eigen::Isometry3d targetInBase(const geometry_msgs::msg::PoseStamped &target);
  enum class PlanRequest { Vision, Manual, Place };
  void plan(PlanRequest request, Trigger::Response &response);
  void checkFreshJoints();
  void publishTargets(const GraspTargets &, const builtin_interfaces::msg::Time &stamp);
  void setStatus(uint8_t phase, uint8_t stage, const std::string &detail);
  void publishStatus();
  void updateJoints(sensor_msgs::msg::JointState::ConstSharedPtr message);
  void previewTarget();
  void prepareExecution();
  void startExecution(bool next, Trigger::Response &response);
  void runExecution();
  void cancel(Trigger::Response &response);
  void checkStart(const moveit_task_constructor_msgs::msg::Solution &solution);
  bool gripperAt(bool closed);
  void checkHardware();
  void updateHardware(serial_driver::msg::HardwareStatus::ConstSharedPtr message);
  TaskSettings settings_;
  Eigen::Isometry3d object_to_tcp_;
  double max_target_age_, max_joint_age_, target_average_max_age_;
  int target_index_, target_average_samples_;
  VisionTargetBuffer vision_history_;
  std::string execution_mode_;
  std::atomic<bool> busy_{false}, cancel_requested_{false}, owned_{false};
  std::mutex input_mutex_, state_mutex_, goal_mutex_, scene_mutex_, command_mutex_;
  Status status_;
  geometry_msgs::msg::PoseArray::ConstSharedPtr vision_;
  geometry_msgs::msg::PoseStamped::ConstSharedPtr manual_;
  sensor_msgs::msg::JointState::ConstSharedPtr joints_;
  serial_driver::msg::HardwareStatus::ConstSharedPtr hardware_;
  double hardware_timeout_ = .1;
  std::chrono::steady_clock::time_point hardware_received_{};
  std::unique_ptr<moveit::task_constructor::Task> task_;
  std::array<moveit_task_constructor_msgs::msg::Solution, 6> stage_solutions_;
  size_t next_stage_ = 0, end_stage_ = 3;
  moveit_msgs::msg::AllowedCollisionMatrix baseline_acm_;
  bool extracted_ = false;
  bool contact_state_needs_review_ = false;
  bool execution_unconfirmed_ = false;
  std::atomic<bool> plan_valid_{false};
  bool preview_visible_ = false;
  std::chrono::steady_clock::time_point last_preview_update_{};
  std::thread worker_;
  std::unique_ptr<EnergyScene> energy_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::CallbackGroup::SharedPtr service_group_;
  rclcpp_action::Client<Execute>::SharedPtr execute_client_;
  GoalHandle::SharedPtr active_goal_;
  rclcpp::Subscription<geometry_msgs::msg::PoseArray>::SharedPtr vision_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr manual_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joints_sub_;
  rclcpp::Subscription<serial_driver::msg::HardwareStatus>::SharedPtr hardware_sub_;
  rclcpp::Service<Trigger>::SharedPtr plan_service_, manual_service_, place_service_, execute_service_,
      next_service_, cancel_service_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr object_pub_, grasp_pub_,
      pregrasp_pub_;
  rclcpp::Publisher<moveit_msgs::msg::DisplayTrajectory>::SharedPtr display_pub_;
  rclcpp::Publisher<Status>::SharedPtr status_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr stage_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr gripper_pub_;
  rclcpp::TimerBase::SharedPtr preview_timer_;
};
} // namespace engineer_mtc
