#pragma once

#include <Eigen/Dense>
#include <geometry_msgs/msg/pose.hpp>
#include <opencv2/core.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <optional>
#include <string>
#include <vector>

namespace vision_core
{
using PointCloud = pcl::PointCloud<pcl::PointXYZ>;

// 默认值来自 26 target_detection/config/vision_params.yaml，距离均为 m。
struct RegistrationConfig
{
  int max_icp_iter{120}, min_points{80}, sor_mean_k{20};
  double voxel_size{0.006}, max_corr_dist{1.0}, sor_std_mul{0.8};
  double max_range{1.5}, bbox_shrink_ratio{0.0};
  double fitness_threshold{0.92}, max_fitness_score{0.01};
  bool centroid_init{true}, use_prev_init{true};
};

struct RegistrationResult
{
  Eigen::Matrix4f transform{Eigen::Matrix4f::Identity()};
  double fitness_score{0.0}, fitness_metric{0.0};
  bool converged{false}, accepted{false};
};

PointCloud::Ptr voxelDownsample(const PointCloud& cloud, double leaf_size);
PointCloud::Ptr removeOutliers(const PointCloud& cloud, int mean_k, double std_mul);
PointCloud::Ptr loadModel(const std::string& path, double scale, int sample_points,
                          double voxel_size);
std::vector<cv::Point2f> projectPoints(const PointCloud& cloud, const cv::Matx33d& camera_matrix);
RegistrationResult registerClouds(const PointCloud::ConstPtr& source,
                                  const PointCloud::ConstPtr& target,
                                  const RegistrationConfig& config,
                                  const std::optional<Eigen::Matrix4f>& initial = std::nullopt);
geometry_msgs::msg::Pose poseFromTransform(const Eigen::Matrix4f& transform);
Eigen::Matrix4f transformFromPose(const geometry_msgs::msg::Pose& pose);
geometry_msgs::msg::Pose zAxisPose(const geometry_msgs::msg::Pose& pose);
}  // namespace vision_core
