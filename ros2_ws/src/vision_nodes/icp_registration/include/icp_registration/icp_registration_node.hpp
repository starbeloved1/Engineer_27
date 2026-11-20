#pragma once

#include "vision_core/pose_constraints.hpp"
#include "vision_core/registration.hpp"
#include <geometry_msgs/msg/pose_array.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <std_msgs/msg/header.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>
#include <vision_msgs/msg/detection2_d_array.hpp>

namespace icp_registration
{
using vision_core::PointCloud;
using vision_core::PoseConstraints;
using vision_core::RegistrationConfig;
class IcpRegistrationNode : public rclcpp::Node
{
 public:
  explicit IcpRegistrationNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

 private:
  void receiveCloud(const sensor_msgs::msg::PointCloud2::ConstSharedPtr& message);
  void publishEmpty(const std_msgs::msg::Header& header);
  void checkTimeout();
  bool fresh(const builtin_interfaces::msg::Time& stamp) const;
  RegistrationConfig config_;
  std::unique_ptr<PoseConstraints> constraints_;
  std::string target_frame_, camera_frame_;
  double max_data_age_{1.5};
  bool publish_tf_{true}, log_debug_{true};
  int log_every_n_{10};
  size_t frame_count_{0};
  bool output_active_{false};
  std_msgs::msg::Header last_header_;
  std::optional<rclcpp::Time> last_input_;
  cv::Matx33d camera_k_{cv::Matx33d::zeros()};
  vision_msgs::msg::Detection2DArray::ConstSharedPtr detections_;
  PointCloud::Ptr source_;
  sensor_msgs::msg::PointCloud2::ConstSharedPtr source_message_;
  Eigen::Matrix4f ply_to_energy_{Eigen::Matrix4f::Identity()};
  std::vector<Eigen::Matrix4f> last_transforms_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_sub_, source_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_sub_;
  rclcpp::Subscription<vision_msgs::msg::Detection2DArray>::SharedPtr detection_sub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr roi_pub_, target_pub_, aligned_pub_,
      raw_aligned_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr pose_pub_, raw_pose_pub_;
  rclcpp::TimerBase::SharedPtr watchdog_;
};
}  // namespace icp_registration
