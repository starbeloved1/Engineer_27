#include "vision_core/registration.hpp"

#include <cmath>
#include <filesystem>
#include <limits>
#include <pcl/common/centroid.h>
#include <pcl/conversions.h>
#include <pcl/filters/random_sample.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/ply_io.h>
#include <pcl/io/vtk_lib_io.h>
#include <pcl/registration/icp.h>
#include <stdexcept>

namespace vision_core
{
PointCloud::Ptr voxelDownsample(const PointCloud& cloud, double leaf_size)
{
  auto result = std::make_shared<PointCloud>();
  if (leaf_size <= 0.0 || cloud.empty())
    return std::make_shared<PointCloud>(cloud);
  pcl::VoxelGrid<pcl::PointXYZ> filter;
  filter.setLeafSize(leaf_size, leaf_size, leaf_size);
  filter.setInputCloud(cloud.makeShared());
  filter.filter(*result);
  return result;
}

PointCloud::Ptr removeOutliers(const PointCloud& cloud, int mean_k, double std_mul)
{
  if (cloud.empty() || mean_k <= 0 || std_mul <= 0.0)
    return std::make_shared<PointCloud>(cloud);
  pcl::StatisticalOutlierRemoval<pcl::PointXYZ> filter;
  filter.setMeanK(mean_k);
  filter.setStddevMulThresh(std_mul);
  filter.setInputCloud(cloud.makeShared());
  auto result = std::make_shared<PointCloud>();
  filter.filter(*result);
  return result;
}

PointCloud::Ptr loadModel(const std::string& path, double scale, int sample_points,
                          double voxel_size)
{
  if (!std::filesystem::is_regular_file(path))
    throw std::runtime_error("模型文件不存在: " + path);
  pcl::PolygonMesh mesh;
  const auto ext = std::filesystem::path(path).extension().string();
  bool loaded = false;
  if (ext == ".stl" || ext == ".STL")
    loaded = pcl::io::loadPolygonFileSTL(path, mesh) > 0;
  else if (ext == ".obj" || ext == ".OBJ")
    loaded = pcl::io::loadPolygonFileOBJ(path, mesh) > 0;
  else if (ext == ".ply" || ext == ".PLY")
    loaded = pcl::io::loadPLYFile(path, mesh) == 0;
  if (!loaded)
    throw std::runtime_error("读取模型失败: " + path);

  PointCloud raw;
  pcl::fromPCLPointCloud2(mesh.cloud, raw);
  auto points = std::make_shared<PointCloud>();
  points->reserve(raw.size());
  // 保持 26 的顺序：读取顶点 -> 缩放 -> 随机限点 -> 米单位体素。
  for (const auto& p : raw)
    points->push_back({static_cast<float>(p.x * scale), static_cast<float>(p.y * scale),
                       static_cast<float>(p.z * scale)});
  if (sample_points > 0 && points->size() > static_cast<size_t>(sample_points))
  {
    pcl::RandomSample<pcl::PointXYZ> filter;
    filter.setSample(sample_points);
    filter.setInputCloud(points);
    auto sampled = std::make_shared<PointCloud>();
    filter.filter(*sampled);
    points = sampled;
  }
  points = voxelDownsample(*points, voxel_size);
  if (points->empty())
    throw std::runtime_error("模型点云为空: " + path);
  return points;
}

std::vector<cv::Point2f> projectPoints(const PointCloud& cloud, const cv::Matx33d& k)
{
  std::vector<cv::Point2f> uv;
  uv.reserve(cloud.size());
  for (const auto& p : cloud)
  {
    if (p.z <= 0)
      uv.emplace_back(std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN());
    else
      uv.emplace_back(static_cast<float>(k(0, 0) * p.x / p.z + k(0, 2)),
                      static_cast<float>(k(1, 1) * p.y / p.z + k(1, 2)));
  }
  return uv;
}

RegistrationResult registerClouds(const PointCloud::ConstPtr& source,
                                  const PointCloud::ConstPtr& target,
                                  const RegistrationConfig& config,
                                  const std::optional<Eigen::Matrix4f>& initial)
{
  RegistrationResult result;
  if (!source || source->empty() || !target || target->empty())
    return result;
  pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
  icp.setInputSource(source);
  icp.setInputTarget(target);
  icp.setMaximumIterations(config.max_icp_iter);
  icp.setMaxCorrespondenceDistance(config.max_corr_dist);
  Eigen::Matrix4f init = initial.value_or(Eigen::Matrix4f::Identity());
  if (!initial && config.flip_initial_direction)
  {
    // 绕模型局部 X 轴旋转 180 度，交换主轴 Z 的正负端。
    init(1, 1) = -1.0f;
    init(2, 2) = -1.0f;
  }
  if (!initial && config.centroid_init)
  {
    // 沿用 26 的 float 累加顺序，不额外设置 ICP 停止条件。
    Eigen::Vector3f a = Eigen::Vector3f::Zero(), b = Eigen::Vector3f::Zero();
    for (const auto& p : *source) a += Eigen::Vector3f(p.x, p.y, p.z);
    for (const auto& p : *target) b += Eigen::Vector3f(p.x, p.y, p.z);
    init.block<3, 1>(0, 3) =
        b / static_cast<float>(target->size()) -
        init.block<3, 3>(0, 0) * (a / static_cast<float>(source->size()));
  }
  PointCloud aligned;
  icp.align(aligned, init);
  result.converged = icp.hasConverged();
  if (!result.converged)
    return result;
  result.transform = icp.getFinalTransformation();
  result.fitness_score = icp.getFitnessScore();  // MSE，m²；默认不复用对应距离阈值。
  result.fitness_metric = 1.0 / (1.0 + result.fitness_score);
  result.accepted =
      result.transform.allFinite() && std::isfinite(result.fitness_score) &&
      result.fitness_metric >= config.fitness_threshold &&
      (config.max_fitness_score <= 0.0 || result.fitness_score <= config.max_fitness_score);
  return result;
}

geometry_msgs::msg::Pose poseFromTransform(const Eigen::Matrix4f& transform)
{
  geometry_msgs::msg::Pose pose;
  pose.position.x = transform(0, 3);
  pose.position.y = transform(1, 3);
  pose.position.z = transform(2, 3);
  const Eigen::Quaternionf q(transform.block<3, 3>(0, 0));
  pose.orientation.x = q.x();
  pose.orientation.y = q.y();
  pose.orientation.z = q.z();
  pose.orientation.w = q.w();
  return pose;
}

Eigen::Matrix4f transformFromPose(const geometry_msgs::msg::Pose& pose)
{
  Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
  const Eigen::Quaternionf q(pose.orientation.w, pose.orientation.x, pose.orientation.y,
                             pose.orientation.z);
  transform.block<3, 3>(0, 0) = q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = Eigen::Vector3f(pose.position.x, pose.position.y, pose.position.z);
  return transform;
}

geometry_msgs::msg::Pose zAxisPose(const geometry_msgs::msg::Pose& input)
{
  auto pose = input;
  const Eigen::Quaternionf q(input.orientation.w, input.orientation.x, input.orientation.y,
                             input.orientation.z);
  Eigen::Vector3f axis = q.toRotationMatrix().col(2);
  Eigen::Quaternionf output = Eigen::Quaternionf::Identity();
  if (axis.norm() > 1e-6f)
    output = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(), axis.normalized());
  pose.orientation.x = output.x();
  pose.orientation.y = output.y();
  pose.orientation.z = output.z();
  pose.orientation.w = output.w();
  return pose;
}
}  // namespace vision_core
