#pragma once
#include "engineer_mtc/grasp_geometry.hpp"
#include <deque>
#include <cmath>
#include <string>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <stdexcept>

namespace engineer_mtc
{
// 只缓存当前下标对应的连续视觉观测；不跨漏检或方向翻转拼接样本。
class VisionTargetBuffer
{
public:
  void push(const geometry_msgs::msg::PoseArray &message, size_t index, size_t required,
            double max_age)
  {
    if (message.header.frame_id.empty() || index >= message.poses.size())
    {
      clear();
      return;
    }
    geometry_msgs::msg::PoseStamped sample;
    sample.header = message.header;
    sample.pose = message.poses[index];
    try { poseTransform(sample.pose); }
    catch (const std::exception &) { clear(); return; }
    if (!samples_.empty())
    {
      const auto &last = samples_.back();
      const double dt = seconds(sample.header.stamp) - seconds(last.header.stamp);
      const Eigen::Quaterniond a(poseTransform(last.pose).linear());
      const Eigen::Quaterniond b(poseTransform(sample.pose).linear());
      if (sample.header.frame_id != last.header.frame_id || message.poses.size() != count_ ||
          dt < 0 || dt > max_age || std::abs(a.dot(b)) < std::sqrt(.5))
        clear();
      else if (dt == 0)
        return;
    }
    count_ = message.poses.size();
    samples_.push_back(sample);
    while (samples_.size() > required) samples_.pop_front();
  }

  std::vector<geometry_msgs::msg::PoseStamped> recent(double now, double max_age,
                                                    size_t required,
                                                    double latest_max_age = -1) const
  {
    if (latest_max_age < 0) latest_max_age = max_age;
    if (samples_.empty() || now - seconds(samples_.back().header.stamp) < -.1 ||
        now - seconds(samples_.back().header.stamp) > latest_max_age)
      throw std::runtime_error("最新视觉目标为空或过期");
    std::vector<geometry_msgs::msg::PoseStamped> result;
    for (const auto &sample : samples_)
    {
      const double age = now - seconds(sample.header.stamp);
      if (age >= -.1 && age <= max_age) result.push_back(sample);
    }
    if (result.size() < required)
      throw std::runtime_error("新鲜视觉样本不足：" + std::to_string(result.size()) + "/" +
                               std::to_string(required) + "，请等待连续观测或调整 target_average_samples");
    result.erase(result.begin(), result.end() - required);
    return result;
  }

private:
  static double seconds(const builtin_interfaces::msg::Time &stamp)
  { return stamp.sec + stamp.nanosec * 1e-9; }
  void clear() { samples_.clear(); count_ = 0; }
  std::deque<geometry_msgs::msg::PoseStamped> samples_;
  size_t count_ = 0;
};
} // namespace engineer_mtc
