#pragma once
#include "engineer_mtc/pregrasp_task.hpp"
#include <atomic>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <mutex>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

namespace engineer_mtc
{
class MtcNode : public rclcpp::Node
{
 public:
  explicit MtcNode(const rclcpp::NodeOptions& options);

 private:
  using Trigger = std_srvs::srv::Trigger;
  geometry_msgs::msg::PoseStamped selectTarget(bool manual);
  void plan(bool manual, Trigger::Response& response);
  void publishTargets(const GraspTargets& targets, const builtin_interfaces::msg::Time& stamp);
  TaskSettings settings_;
  Eigen::Isometry3d object_to_tcp_;
  double max_target_age_, max_joint_age_;
  int target_index_;
  std::atomic<bool> busy_{false};
  std::mutex input_mutex_;
  geometry_msgs::msg::PoseArray::ConstSharedPtr vision_;
  geometry_msgs::msg::PoseStamped::ConstSharedPtr manual_;
  sensor_msgs::msg::JointState::ConstSharedPtr joints_;
  std::unique_ptr<moveit::task_constructor::Task> task_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::CallbackGroup::SharedPtr service_group_;
  rclcpp::Subscription<geometry_msgs::msg::PoseArray>::SharedPtr vision_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr manual_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joints_sub_;
  rclcpp::Service<Trigger>::SharedPtr plan_service_, manual_service_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr object_pub_, grasp_pub_, pregrasp_pub_;
  rclcpp::Publisher<moveit_msgs::msg::DisplayTrajectory>::SharedPtr display_pub_;
};
}  // namespace engineer_mtc
