#pragma once
#include "vision_nodes_pcl/registration.hpp"
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace vision_nodes_pcl
{
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
}  // namespace vision_nodes_pcl
