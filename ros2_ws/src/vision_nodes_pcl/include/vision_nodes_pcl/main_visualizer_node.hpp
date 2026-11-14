#pragma once
#include <chrono>
#include <optional>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <vision_msgs/msg/detection2_d_array.hpp>

namespace vision_nodes_pcl
{
class MainVisualizerNode : public rclcpp::Node
{
 public:
  MainVisualizerNode();

 private:
  using Image = sensor_msgs::msg::Image;
  using Detections = vision_msgs::msg::Detection2DArray;
  void render(const Image::ConstSharedPtr& image, const Detections::ConstSharedPtr& detections);
  double period_{0.0};
  std::optional<std::chrono::steady_clock::time_point> last_render_;
  rclcpp::Publisher<Image>::SharedPtr publisher_;
  rclcpp::Subscription<Image>::SharedPtr image_sub_;
  message_filters::Subscriber<Image> synced_image_;
  message_filters::Subscriber<Detections> synced_detections_;
  std::shared_ptr<message_filters::TimeSynchronizer<Image, Detections>> synchronizer_;
};
}  // namespace vision_nodes_pcl
