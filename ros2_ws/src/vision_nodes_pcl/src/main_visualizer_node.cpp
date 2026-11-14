#include "vision_nodes_pcl/main_visualizer_node.hpp"
#include <algorithm>
#include <cmath>
#include <cv_bridge/cv_bridge.h>
#include <iomanip>
#include <opencv2/imgproc.hpp>
#include <sstream>

namespace vision_nodes_pcl
{
MainVisualizerNode::MainVisualizerNode() : Node("main_visualizer")
{
  const auto color_topic =
      declare_parameter<std::string>("color_topic", "/camera/camera/color/image_raw");
  const auto detection_topic =
      declare_parameter<std::string>("detection_topic", "/vision/yolo/detections");
  const auto output_topic =
      declare_parameter<std::string>("visualization_topic", "/vision/visualization/image");
  const auto draw = declare_parameter<bool>("draw_detections", true);
  const auto queue_size = declare_parameter<int>("sync_queue_size", 30);
  const auto rate = declare_parameter<double>("visualization_rate_hz", 0.0);
  const auto reliability = declare_parameter<std::string>("input_reliability", "reliable");
  if (color_topic.empty() || detection_topic.empty() || output_topic.empty() ||
      get_node_base_interface()->resolve_topic_or_service_name(color_topic, false) ==
          get_node_base_interface()->resolve_topic_or_service_name(output_topic, false) ||
      queue_size <= 0 || !std::isfinite(rate) || rate < 0 ||
      (reliability != "reliable" && reliability != "best_effort"))
    throw std::invalid_argument("可视化话题、频率、队列或 QoS 参数无效");
  period_ = rate > 0 ? 1.0 / rate : 0.0;
  publisher_ = create_publisher<Image>(output_topic, 1);
  auto qos = rclcpp::QoS(1);
  if (reliability == "best_effort")
    qos.best_effort();
  if (draw)
  {
    synced_image_.subscribe(this, color_topic, qos.get_rmw_qos_profile());
    synced_detections_.subscribe(this, detection_topic, rclcpp::QoS(10).get_rmw_qos_profile());
    synchronizer_ = std::make_shared<message_filters::TimeSynchronizer<Image, Detections>>(
        synced_image_, synced_detections_, queue_size);
    synchronizer_->registerCallback(
        std::bind(&MainVisualizerNode::render, this, std::placeholders::_1, std::placeholders::_2));
  }
  else
    image_sub_ = create_subscription<Image>(
        color_topic, qos, [this](Image::ConstSharedPtr image) { render(image, nullptr); });
  RCLCPP_INFO(get_logger(), "Main visualizer: %s -> %s, draw=%d", color_topic.c_str(),
              output_topic.c_str(), draw);
}

void MainVisualizerNode::render(const Image::ConstSharedPtr& image,
                                const Detections::ConstSharedPtr& detections)
{
  if (publisher_->get_subscription_count() == 0)
    return;
  const auto current = std::chrono::steady_clock::now();
  if (last_render_ && std::chrono::duration<double>(current - *last_render_).count() < period_)
    return;
  if (!detections)
    publisher_->publish(*image);
  else
  {
    if (image->header.frame_id != detections->header.frame_id)
    {
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "检测框与彩色图像 frame_id 不一致");
      return;
    }
    try
    {
      auto output = cv_bridge::toCvCopy(image, "bgr8");
      auto& frame = output->image;
      for (const auto& detection : detections->detections)
      {
        if (detection.results.empty())
          continue;
        const auto hypothesis = std::max_element(
            detection.results.begin(), detection.results.end(),
            [](const auto& a, const auto& b) { return a.hypothesis.score < b.hypothesis.score; });
        const auto& box = detection.bbox;
        const double left = box.center.position.x - box.size_x / 2;
        const double top = box.center.position.y - box.size_y / 2;
        const double right = box.center.position.x + box.size_x / 2;
        const double bottom = box.center.position.y + box.size_y / 2;
        const auto score = hypothesis->hypothesis.score;
        if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) ||
            !std::isfinite(bottom) || !std::isfinite(score) || score < 0 || score > 1 ||
            right <= left || bottom <= top || right < 0 || bottom < 0 || left >= frame.cols ||
            top >= frame.rows)
          continue;
        const cv::Point a(static_cast<int>(std::lround(std::clamp(left, 0.0, frame.cols - 1.0))),
                          static_cast<int>(std::lround(std::clamp(top, 0.0, frame.rows - 1.0))));
        const cv::Point b(static_cast<int>(std::lround(std::clamp(right, 0.0, frame.cols - 1.0))),
                          static_cast<int>(std::lround(std::clamp(bottom, 0.0, frame.rows - 1.0))));
        cv::rectangle(frame, a, b, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
        std::ostringstream label;
        label << hypothesis->hypothesis.class_id << ": " << std::fixed << std::setprecision(2)
              << score;
        cv::putText(frame, label.str(), cv::Point(a.x, std::max(16, a.y - 6)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
      }
      publisher_->publish(*output->toImageMsg());
    }
    catch (const cv_bridge::Exception& error)
    {
      RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), 5000, "图像转换失败: %s", error.what());
      return;
    }
    catch (const cv::Exception& error)
    {
      RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), 5000, "图像绘制失败: %s", error.what());
      return;
    }
  }
  last_render_ = current;
}
}  // namespace vision_nodes_pcl
