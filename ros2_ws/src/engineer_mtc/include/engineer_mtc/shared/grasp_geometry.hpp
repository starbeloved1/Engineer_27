#pragma once
#include <Eigen/Geometry>
#include <geometry_msgs/msg/pose.hpp>
#include <vector>

namespace engineer_mtc
{
// 工具函数
Eigen::Isometry3d poseTransform(const geometry_msgs::msg::Pose& pose);
geometry_msgs::msg::Pose transformPose(const Eigen::Isometry3d& transform);
Eigen::Isometry3d xyzRpyTransform(const std::vector<double>& xyz, const std::vector<double>& rpy);

/**
 * @brief 对同一坐标系下的多帧位姿求平均。
 * @param poses 非空的有效位姿序列。
 * @return 位置算术平均、朝向四元数平均得到的位姿。
 */
geometry_msgs::msg::Pose averagePoses(const std::vector<geometry_msgs::msg::Pose> &poses);

/// 本次取矿使用标准目标格式
struct GraspTargets
{
  Eigen::Isometry3d object_base;       ///< 矿模型原点位姿。
  Eigen::Isometry3d grasp_tcp_base;    ///< 抓取 TCP 目标位姿。
  Eigen::Isometry3d pregrasp_tip_base; ///< 预抓取 link_6 目标位姿。
};

/**
 * @brief 根据矿位姿和夹持几何计算抓取与预抓取目标。
 * @param object_base 基座到矿的变换。
 * @param object_to_tcp 矿到抓取 TCP 的变换。
 * @param tip_to_tcp link_6 到 TCP 的变换。
 * @param approach_axis TCP 坐标系下的接近方向。
 * @param distance 沿接近方向反向退开的预抓取距离，单位为米。
 * @return 基座下的矿、抓取 TCP 和预抓取 link_6 位姿。
 */
GraspTargets computeGraspTargets(const Eigen::Isometry3d& object_base,
                                const Eigen::Isometry3d& object_to_tcp,
                                const Eigen::Isometry3d& tip_to_tcp,
                                const Eigen::Vector3d& approach_axis, double distance);
}  // namespace engineer_mtc
