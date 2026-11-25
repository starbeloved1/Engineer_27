#include "engineer_mtc/shared/grasp_geometry.hpp"
#include <cmath>
#include <Eigen/Eigenvalues>
#include <stdexcept>

namespace engineer_mtc
{
Eigen::Isometry3d poseTransform(const geometry_msgs::msg::Pose& pose)
{
  Eigen::Quaterniond q(pose.orientation.w, pose.orientation.x, pose.orientation.y,
                       pose.orientation.z);
  Eigen::Vector3d p(pose.position.x, pose.position.y, pose.position.z);
  if (!p.allFinite() || !q.coeffs().allFinite() || std::abs(q.norm() - 1.0) > 0.01)
    throw std::invalid_argument("目标位置须有限，四元数须为单位四元数");
  Eigen::Isometry3d result = Eigen::Isometry3d::Identity();
  result.translation() = p;
  result.linear() = q.normalized().toRotationMatrix();
  return result;
}

geometry_msgs::msg::Pose transformPose(const Eigen::Isometry3d& transform)
{
  geometry_msgs::msg::Pose pose;
  pose.position.x = transform.translation().x();
  pose.position.y = transform.translation().y();
  pose.position.z = transform.translation().z();
  const Eigen::Quaterniond q(transform.linear());
  pose.orientation.x = q.x();
  pose.orientation.y = q.y();
  pose.orientation.z = q.z();
  pose.orientation.w = q.w();
  return pose;
}

Eigen::Isometry3d xyzRpyTransform(const std::vector<double>& xyz, const std::vector<double>& rpy)
{
  if (xyz.size() != 3 || rpy.size() != 3)
    throw std::invalid_argument("xyz 与 rpy 均须为三个数，单位 m / rad");
  for (const auto* values : {&xyz, &rpy})
    for (const double v : *values)
      if (!std::isfinite(v)) throw std::invalid_argument("抓取偏移必须为有限值");
  Eigen::Isometry3d result = Eigen::Isometry3d::Identity();
  result.translation() = Eigen::Vector3d(xyz[0], xyz[1], xyz[2]);
  result.linear() = (Eigen::AngleAxisd(rpy[2], Eigen::Vector3d::UnitZ()) *
                     Eigen::AngleAxisd(rpy[1], Eigen::Vector3d::UnitY()) *
                     Eigen::AngleAxisd(rpy[0], Eigen::Vector3d::UnitX())).toRotationMatrix();
  return result;
}


geometry_msgs::msg::Pose averagePoses(const std::vector<geometry_msgs::msg::Pose> &poses)
{
  if (poses.empty()) throw std::invalid_argument("不能平均空位姿窗口");
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
  Eigen::Matrix4d scatter = Eigen::Matrix4d::Zero();
  for (const auto &pose : poses)
  {
    const auto transform = poseTransform(pose);
    position += transform.translation();
    const Eigen::Quaterniond q(transform.linear());
    scatter += q.coeffs() * q.coeffs().transpose();
  }
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix4d> solver(scatter);
  if (solver.info() != Eigen::Success ||
      solver.eigenvalues()[3] - solver.eigenvalues()[2] < 1e-8 * poses.size())
    throw std::runtime_error("视觉朝向均值不唯一，请重新采样");
  Eigen::Quaterniond q;
  q.coeffs() = solver.eigenvectors().col(3);
  Eigen::Isometry3d mean = Eigen::Isometry3d::Identity();
  mean.translation() = position / static_cast<double>(poses.size());
  mean.linear() = q.normalized().toRotationMatrix();
  return transformPose(mean);
}

// 打包 GraspTargets
GraspTargets computeGraspTargets(const Eigen::Isometry3d& object_base,
                                const Eigen::Isometry3d& object_to_tcp,
                                const Eigen::Isometry3d& tip_to_tcp,
                                const Eigen::Vector3d& approach_axis, double distance)
{
  if (!approach_axis.allFinite() || approach_axis.norm() < 1e-9 ||
      !std::isfinite(distance) || distance <= 0)
    throw std::invalid_argument("接近方向须非零，预抓取距离须为有限正数");
  GraspTargets targets;
  targets.object_base = object_base;
  targets.grasp_tcp_base = object_base * object_to_tcp;
  Eigen::Isometry3d retreat = Eigen::Isometry3d::Identity();
  retreat.translation() = -distance * approach_axis.normalized();
  // 先在 TCP 局部方向减去，再换算成规划末端 link_6
  targets.pregrasp_tip_base = targets.grasp_tcp_base * retreat * tip_to_tcp.inverse();
  return targets;
}
}  // namespace engineer_mtc
