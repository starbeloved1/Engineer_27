#pragma once
#include <Eigen/Geometry>
#include <geometry_msgs/msg/pose.hpp>
#include <vector>

namespace engineer_mtc
{
// 所有变换用 T_parent_child 表示：把 child 坐标转换到 parent
Eigen::Isometry3d poseTransform(const geometry_msgs::msg::Pose& pose);
geometry_msgs::msg::Pose transformPose(const Eigen::Isometry3d& transform);
Eigen::Isometry3d xyzRpyTransform(const std::vector<double>& xyz, const std::vector<double>& rpy);

//1.base下矿位姿
//2.base_tcp
//3.预抓取位姿
struct GraspTargets
{
  Eigen::Isometry3d object_base, grasp_tcp_base, pregrasp_tip_base;
};

GraspTargets computeGraspTargets(const Eigen::Isometry3d& object_base,
                                const Eigen::Isometry3d& object_to_tcp,
                                const Eigen::Isometry3d& tip_to_tcp,
                                const Eigen::Vector3d& approach_axis, double distance);
}  // namespace engineer_mtc
