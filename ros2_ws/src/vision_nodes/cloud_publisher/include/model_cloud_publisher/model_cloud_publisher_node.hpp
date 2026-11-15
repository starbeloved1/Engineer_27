#pragma once
#include "vision_core/registration.hpp"
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace model_cloud_publisher
{
using vision_core::PointCloud;
class ModelCloudPublisherNode : public rclcpp::Node
{
 public:
  ModelCloudPublisherNode();

 private:
  void publishCloud();
  PointCloud::Ptr points_;
  std::string frame_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};
}  // namespace model_cloud_publisher
