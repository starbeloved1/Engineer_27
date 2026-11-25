#pragma once
#include <cstddef>
#include <deque>
#include <vector>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

namespace engineer_mtc
{
// 只缓存当前下标对应的连续视觉观测；不跨漏检或方向翻转拼接样本。
class VisionTargetBuffer
{
public:
  /**
   * @brief 缓存指定目标下标的连续有效观测。
   * @param message 当前帧的矿位姿数组。
   * @param index 目标在数组中的下标。
   * @param required 保留的最大样本数。
   * @param max_age 相邻观测允许的最大间隔，单位为秒。
   * @note 无效或不连续观测会清空窗口，重复时间戳跳过。
   */
  void push(const geometry_msgs::msg::PoseArray &message, size_t index, size_t required,
            double max_age);

  /**
   * @brief 获取满足时效与数量要求的最新样本副本。
   * @param now 当前 ROS 时间，单位为秒。
   * @param max_age 历史样本的最大年龄，单位为秒。
   * @param required 本次需要的样本数。
   * @param latest_max_age 最新样本的最大年龄，负数表示沿用 max_age。
   * @return 按时间排列的 required 个样本。
   */
  std::vector<geometry_msgs::msg::PoseStamped> recent(double now, double max_age,
                                                   size_t required,
                                                   double latest_max_age = -1) const;

private:
  /// 将 ROS 时间戳转换为秒。
  static double seconds(const builtin_interfaces::msg::Time &stamp);

  /// 清空观测窗口和目标数量记录。
  void clear();

  std::deque<geometry_msgs::msg::PoseStamped> samples_;
  size_t count_ = 0;
};
} // namespace engineer_mtc
