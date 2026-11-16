#include "model_cloud_publisher/model_cloud_publisher_node.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <cmath>
#include <filesystem>
#include <pcl_conversions/pcl_conversions.h>

namespace model_cloud_publisher
{
using namespace vision_core;
ModelCloudPublisherNode::ModelCloudPublisherNode() : Node("model_cloud_publisher")
{
  auto path = declare_parameter<std::string>("model_path", "assets/energy/energy.ply");
  const auto scale = declare_parameter<double>("scale", 0.001);
  const auto samples = declare_parameter<int>("sample_points", 5000);
  const auto voxel = declare_parameter<double>("voxel_size", 0.005);
  const auto rate = declare_parameter<double>("publish_rate", 100.0);
  frame_ = declare_parameter<std::string>("frame_id", "energy_model");
  const auto topic = declare_parameter<std::string>("topic", "/vision/model_cloud");
  if (!std::isfinite(scale) || scale <= 0 || samples < 0 || !std::isfinite(voxel) || voxel < 0 ||
      !std::isfinite(rate) || rate <= 0 || frame_.empty() || topic.empty())
    throw std::invalid_argument("模型缩放、采样数、体素、发布频率或话题/坐标系无效");
  if (std::filesystem::path(path).is_relative())
    path =
        (std::filesystem::path(ament_index_cpp::get_package_share_directory("model_cloud_publisher")) /
         path)
            .string();
  points_ = loadModel(path, scale, samples, voxel);
  // 保留 26 的 100 Hz 发布，同时让后加入的显示端能读到模型。
  publisher_ =
      create_publisher<sensor_msgs::msg::PointCloud2>(topic, rclcpp::QoS(1).transient_local());
  timer_ = create_wall_timer(std::chrono::duration<double>(1.0 / rate),
                             std::bind(&ModelCloudPublisherNode::publishCloud, this));
  publishCloud();
  RCLCPP_INFO(get_logger(), "PCL model: %s, %zu points, %.1f Hz, CAD frame=%s", path.c_str(),
              points_->size(), rate, frame_.c_str());
}

void ModelCloudPublisherNode::publishCloud()
{
  sensor_msgs::msg::PointCloud2 message;
  pcl::toROSMsg(*points_, message);
  message.header.frame_id = frame_;
  message.header.stamp = now();
  publisher_->publish(message);
}
}  // namespace model_cloud_publisher
