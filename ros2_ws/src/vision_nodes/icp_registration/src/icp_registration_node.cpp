#include "icp_registration/icp_registration_node.hpp"

#include <algorithm>
#include <cmath>
#include <pcl/common/transforms.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl_conversions/pcl_conversions.h>
#include <tf2_eigen/tf2_eigen.hpp>

namespace icp_registration
{
using namespace vision_core;
namespace
{
using ColorCloud = pcl::PointCloud<pcl::PointXYZRGB>;

bool isLockedRoi(const vision_msgs::msg::Detection2DArray& detections)
{
  return !detections.detections.empty() &&
         std::all_of(detections.detections.begin(), detections.detections.end(),
                     [](const auto& detection) { return detection.id == "__locked_yolo_roi__"; });
}

template <class Cloud>
sensor_msgs::msg::PointCloud2 cloudMessage(const Cloud& cloud, const std_msgs::msg::Header& header)
{
  sensor_msgs::msg::PointCloud2 message;
  pcl::toROSMsg(cloud, message);
  message.header = header;
  return message;
}

// RGB 只用于显示；过滤和 ICP 始终使用 26 的 PointXYZ。
ColorCloud attachColors(const PointCloud& filtered, const PointCloud::Ptr& roi,
                        const ColorCloud& colors)
{
  ColorCloud output;
  if (filtered.empty() || roi->empty())
    return output;
  pcl::KdTreeFLANN<pcl::PointXYZ> tree;
  tree.setInputCloud(roi);
  std::vector<int> indices(1);
  std::vector<float> distances(1);
  for (const auto& point : filtered)
  {
    pcl::PointXYZRGB color;
    color.x = point.x;
    color.y = point.y;
    color.z = point.z;
    color.r = color.g = color.b = 180;
    if (tree.nearestKSearch(point, 1, indices, distances) > 0)
      color.rgba = colors[indices[0]].rgba;
    output.push_back(color);
  }
  return output;
}
}  // namespace

IcpRegistrationNode::IcpRegistrationNode(const rclcpp::NodeOptions& options)
    : Node("icp_registration", options)
{
  const auto cloud_topic =
      declare_parameter<std::string>("pointcloud_topic", "/camera/camera/depth/color/points");
  const auto camera_topic =
      declare_parameter<std::string>("camera_info_topic", "/camera/camera/color/camera_info");
  const auto detection_topic =
      declare_parameter<std::string>("detection_topic", "/vision/yolo/detections");
  const auto source_topic =
      declare_parameter<std::string>("source_cloud_topic", "/vision/model_cloud");
  const auto roi_topic = declare_parameter<std::string>("roi_cloud_topic", "/vision/icp/roi_cloud");
  const auto target_topic =
      declare_parameter<std::string>("target_cloud_topic", "/vision/icp/target_cloud");
  const auto aligned_topic =
      declare_parameter<std::string>("aligned_cloud_topic", "/vision/icp/aligned_cloud");
  const auto raw_aligned_topic =
      declare_parameter<std::string>("raw_aligned_cloud_topic", "/vision/icp/raw_aligned_cloud");
  const auto pose_topic = declare_parameter<std::string>("pose_topic", "/vision/icp/object_poses");
  const auto raw_pose_topic =
      declare_parameter<std::string>("raw_pose_topic", "/vision/icp/raw_object_poses");
  const auto energy_xyz = declare_parameter<std::vector<double>>(
      "ply_to_energy_xyz", {.000734246038, -.001457943306, .004333127084});
  const auto energy_rpy = declare_parameter<std::vector<double>>(
      "ply_to_energy_rpy", {3.139041892859, -.000000116949, -.000003732359});
  if (energy_xyz.size() != 3 || energy_rpy.size() != 3 ||
      !std::all_of(energy_xyz.begin(), energy_xyz.end(), [](double v) { return std::isfinite(v); }) ||
      !std::all_of(energy_rpy.begin(), energy_rpy.end(), [](double v) { return std::isfinite(v); }))
    throw std::invalid_argument("ply_to_energy_xyz/rpy 须为三个有限数，单位 m/rad");
  // T_PLY_energy：仅在对外发布位姿时应用；配准、历史初值及点云后处理仍使用 PLY。
  ply_to_energy_.block<3, 3>(0, 0) =
      (Eigen::AngleAxisd(energy_rpy[2], Eigen::Vector3d::UnitZ()) *
       Eigen::AngleAxisd(energy_rpy[1], Eigen::Vector3d::UnitY()) *
       Eigen::AngleAxisd(energy_rpy[0], Eigen::Vector3d::UnitX())).toRotationMatrix().cast<float>();
  ply_to_energy_.block<3, 1>(0, 3) =
      Eigen::Vector3d(energy_xyz[0], energy_xyz[1], energy_xyz[2]).cast<float>();
  target_frame_ = declare_parameter<std::string>("target_frame", "camera_link");
  max_data_age_ = declare_parameter<double>("max_data_age", 1.5);
  publish_tf_ = declare_parameter<bool>("publish_tf", true);
  log_debug_ = declare_parameter<bool>("log_debug", true);
  log_every_n_ = declare_parameter<int>("log_every_n", 10);
  const auto reliability = declare_parameter<std::string>("input_reliability", "reliable");
  if (reliability != "reliable" && reliability != "best_effort")
    throw std::invalid_argument("input_reliability 必须是 reliable 或 best_effort");
  auto parameter = [this](const char* name, auto& value)
  { value = declare_parameter<std::decay_t<decltype(value)>>(name, value); };
#define READ_CONFIG(field) parameter(#field, config_.field)
  READ_CONFIG(max_icp_iter);
  READ_CONFIG(voxel_size);
  READ_CONFIG(max_corr_dist);
  READ_CONFIG(min_points);
  READ_CONFIG(sor_mean_k);
  READ_CONFIG(sor_std_mul);
  READ_CONFIG(max_range);
  READ_CONFIG(bbox_shrink_ratio);
  READ_CONFIG(fitness_threshold);
  READ_CONFIG(max_fitness_score);
  READ_CONFIG(centroid_init);
  READ_CONFIG(flip_initial_direction);
  READ_CONFIG(use_prev_init);
#undef READ_CONFIG
  ConstraintConfig constraint_config;
#define READ_CONFIG(field) parameter(#field, constraint_config.field)
  READ_CONFIG(use_hexagon_prior);
  READ_CONFIG(hexagon_vertex_count);
  READ_CONFIG(hexagon_min_observations);
  READ_CONFIG(hexagon_nominal_radius);
  READ_CONFIG(hexagon_radius_tolerance);
  READ_CONFIG(hexagon_snap_max_distance);
  READ_CONFIG(hexagon_snap_gain);
  READ_CONFIG(hexagon_state_smoothing);
  READ_CONFIG(force_outward_z_axis);
  READ_CONFIG(enforce_z_axis_angle_constraint);
  READ_CONFIG(z_axis_angle_constraint_gain);
  READ_CONFIG(z_axis_angle_max_deviation_deg);
#undef READ_CONFIG
  for (const auto value :
       {config_.voxel_size, config_.max_corr_dist, config_.sor_std_mul, config_.max_range,
        config_.bbox_shrink_ratio, config_.fitness_threshold, config_.max_fitness_score,
        max_data_age_, constraint_config.hexagon_nominal_radius,
        constraint_config.hexagon_radius_tolerance, constraint_config.hexagon_snap_max_distance,
        constraint_config.hexagon_snap_gain, constraint_config.hexagon_state_smoothing,
        constraint_config.z_axis_angle_constraint_gain,
        constraint_config.z_axis_angle_max_deviation_deg})
    if (!std::isfinite(value) || value < 0)
      throw std::invalid_argument("配准与先验参数必须为有限非负值");
  if (config_.max_icp_iter <= 0 || config_.max_corr_dist <= 0 || config_.min_points < 3 ||
      config_.sor_mean_k < 0 || config_.bbox_shrink_ratio > 0.45 || config_.fitness_threshold > 1 ||
      max_data_age_ <= 0 || log_every_n_ < 0 || constraint_config.hexagon_vertex_count < 3 ||
      constraint_config.hexagon_min_observations < 2)
    throw std::invalid_argument("迭代数、点数、距离、框收缩、分数、超时或六矿布局参数无效");
  constraints_ = std::make_unique<PoseConstraints>(constraint_config);
  pose_pub_ = create_publisher<geometry_msgs::msg::PoseArray>(pose_topic, 10);
  raw_pose_pub_ = create_publisher<geometry_msgs::msg::PoseArray>(raw_pose_topic, 10);
  roi_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(roi_topic, 1);
  target_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(target_topic, 1);
  aligned_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(aligned_topic, 1);
  raw_aligned_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(raw_aligned_topic, 1);
  tf_buffer_ = std::make_unique<tf2_ros::Buffer>(get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
  if (publish_tf_)
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
  // 26 的输入策略：最新检测框 + 当前点云。
  // 处理跟不上采集时丢弃旧帧，避免旧点云累积增加端到端延迟。
  auto input_qos = rclcpp::QoS(1);
  if (reliability == "best_effort")
    input_qos.best_effort();
  cloud_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
      cloud_topic, input_qos,
      std::bind(&IcpRegistrationNode::receiveCloud, this, std::placeholders::_1));
  camera_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
      camera_topic, input_qos,
      [this](sensor_msgs::msg::CameraInfo::ConstSharedPtr message)
      {
        for (int i = 0; i < 9; ++i) camera_k_.val[i] = message->k[i];
        camera_frame_ = message->header.frame_id;
      });
  // 检测框独立调度，耗时的点云配准不阻塞接收最新框。
  detection_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);
  rclcpp::SubscriptionOptions detection_options;
  detection_options.callback_group = detection_group_;
  detection_sub_ = create_subscription<vision_msgs::msg::Detection2DArray>(
      detection_topic, rclcpp::QoS(1),
      [this](vision_msgs::msg::Detection2DArray::ConstSharedPtr message)
      {
        std::lock_guard<std::mutex> lock(detections_mutex_);
        detections_ = message;
      }, detection_options);
  source_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
      source_topic, 1,
      [this](sensor_msgs::msg::PointCloud2::ConstSharedPtr message)
      {
        if (source_message_ && source_message_->width == message->width &&
            source_message_->height == message->height &&
            source_message_->fields == message->fields &&
            source_message_->is_bigendian == message->is_bigendian &&
            source_message_->point_step == message->point_step &&
            source_message_->row_step == message->row_step &&
            source_message_->data == message->data)
          return;
        PointCloud cloud;
        pcl::fromROSMsg(*message, cloud);
        source_ = voxelDownsample(cloud, config_.voxel_size);
        source_message_ = message;
      });
  watchdog_ = create_wall_timer(std::chrono::milliseconds(100),
                                std::bind(&IcpRegistrationNode::checkTimeout, this));
  RCLCPP_INFO(get_logger(),
              "26 PCL pipeline ready; target_frame=%s, max_corr_dist=%.3fm, max_score=%.5fm², "
              "flip_initial_direction=%s",
              target_frame_.c_str(), config_.max_corr_dist, config_.max_fitness_score,
              config_.flip_initial_direction ? "true" : "false");
}

bool IcpRegistrationNode::fresh(const builtin_interfaces::msg::Time& stamp) const
{
  const auto age = (now() - rclcpp::Time(stamp, get_clock()->get_clock_type())).seconds();
  return age >= -0.1 && age <= max_data_age_;
}

void IcpRegistrationNode::publishEmpty(const std_msgs::msg::Header& header)
{
  geometry_msgs::msg::PoseArray poses;
  poses.header = header;
  pose_pub_->publish(poses);
  raw_pose_pub_->publish(poses);
  roi_pub_->publish(cloudMessage(ColorCloud{}, header));
  target_pub_->publish(cloudMessage(ColorCloud{}, header));
  aligned_pub_->publish(cloudMessage(PointCloud{}, header));
  raw_aligned_pub_->publish(cloudMessage(PointCloud{}, header));
  output_active_ = false;
}

void IcpRegistrationNode::checkTimeout()
{
  if (!last_input_)
    return;
  const auto elapsed = (now() - *last_input_).seconds();
  if (elapsed >= 0 && elapsed <= max_data_age_)
    return;
  if (output_active_)
  {
    auto header = last_header_;
    header.stamp = now();
    publishEmpty(header);
  }
  {
    std::lock_guard<std::mutex> lock(detections_mutex_);
    if (detections_ && !isLockedRoi(*detections_) && !fresh(detections_->header.stamp))
      detections_.reset();
  }
  last_transforms_.clear();
  constraints_->reset();
  last_input_.reset();
}

void IcpRegistrationNode::receiveCloud(const sensor_msgs::msg::PointCloud2::ConstSharedPtr& message)
{
  ++frame_count_;
  const bool log = log_debug_ && log_every_n_ > 0 && frame_count_ % log_every_n_ == 0;
  auto header = message->header;
  if (!target_frame_.empty())
    header.frame_id = target_frame_;
  last_header_ = header;
  vision_msgs::msg::Detection2DArray::ConstSharedPtr detections;
  {
    std::lock_guard<std::mutex> lock(detections_mutex_);
    detections = detections_;
  }
  if (!detections || !source_ || source_->empty() || camera_k_(2, 2) == 0 ||
      !fresh(message->header.stamp) ||
      (!isLockedRoi(*detections) && !fresh(detections->header.stamp)))
  {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 3000,
        "配准输入不可用: detection=%d model=%d camera=%d cloud_age=%.3fs detection_age=%.3fs limit=%.3fs",
        static_cast<int>(!!detections), static_cast<int>(source_ && !source_->empty()),
        static_cast<int>(camera_k_(2, 2) != 0),
        (now() - rclcpp::Time(message->header.stamp, get_clock()->get_clock_type())).seconds(),
        detections ? (now() - rclcpp::Time(detections->header.stamp, get_clock()->get_clock_type())).seconds() : -1.0,
        max_data_age_);
    publishEmpty(header);
    return;
  }
  last_input_ = now();
  if (detections->detections.empty())
  {
    publishEmpty(header);
    return;
  }
  if (camera_frame_.empty() || detections->header.frame_id != camera_frame_)
  {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                        "检测框与 CameraInfo 坐标系不一致或为空: detections=%s, camera=%s",
                        detections->header.frame_id.c_str(), camera_frame_.c_str());
    publishEmpty(header);
    return;
  }
  PointCloud input;
  pcl::fromROSMsg(*message, input);
  const bool has_color =
      std::any_of(message->fields.begin(), message->fields.end(),
                  [](const auto& field) { return field.name == "rgb" || field.name == "rgba"; });
  ColorCloud input_colors;
  if (has_color)
    pcl::fromROSMsg(*message, input_colors);
  PointCloud cloud;
  ColorCloud colors;
  for (size_t i = 0; i < input.size(); ++i)
  {
    const auto& p = input[i];
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z))
      continue;
    const double distance =
        std::sqrt(static_cast<double>(p.x) * p.x + static_cast<double>(p.y) * p.y +
                  static_cast<double>(p.z) * p.z);
    if (config_.max_range > 0 && distance > config_.max_range)
      continue;
    cloud.push_back(p);
    pcl::PointXYZRGB color;
    if (has_color)
      color = input_colors[i];
    else
    {
      color.x = p.x;
      color.y = p.y;
      color.z = p.z;
      color.r = color.g = color.b = 180;
    }
    colors.push_back(color);
  }
  Eigen::Matrix4f to_camera = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f to_target = Eigen::Matrix4f::Identity();
  try
  {
    if (camera_frame_ != message->header.frame_id)
    {
      const auto transform =
          tf_buffer_->lookupTransform(camera_frame_, message->header.frame_id,
                                      message->header.stamp, rclcpp::Duration::from_seconds(0.1));
      to_camera = tf2::transformToEigen(transform).matrix().cast<float>();
    }
    if (!target_frame_.empty() && target_frame_ != message->header.frame_id)
    {
      const auto transform =
          tf_buffer_->lookupTransform(target_frame_, message->header.frame_id,
                                      message->header.stamp, rclcpp::Duration::from_seconds(0.1));
      to_target = tf2::transformToEigen(transform).matrix().cast<float>();
    }
  }
  catch (const tf2::TransformException& error)
  {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "投影或输出坐标系 TF 不可用: %s",
                         error.what());
    publishEmpty(header);
    return;
  }
  // 投影使用 CameraInfo 所属光学坐标系；框内取点仍保留输入 XYZ，随后统一转到 target_frame。
  // RGB 纹理和 align_depth 不意味着原始点云 XYZ 已在彩色坐标系。
  PointCloud projection_cloud;
  pcl::transformPointCloud(cloud, projection_cloud, to_camera);
  const auto uv = projectPoints(projection_cloud, camera_k_);
  ColorCloud roi_all, target_all;
  PointCloud raw_aligned_all, aligned_all;
  geometry_msgs::msg::PoseArray raw_poses, poses;
  raw_poses.header = poses.header = header;
  std::vector<PoseCandidate> candidates;
  for (size_t index = 0; index < detections->detections.size(); ++index)
  {
    const auto& bbox = detections->detections[index].bbox;
    const double cx = bbox.center.position.x, cy = bbox.center.position.y;
    const double w = bbox.size_x * (1.0 - 2.0 * config_.bbox_shrink_ratio);
    const double h = bbox.size_y * (1.0 - 2.0 * config_.bbox_shrink_ratio);
    if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(w) || !std::isfinite(h) ||
        w <= 0 || h <= 0)
      continue;
    auto roi = std::make_shared<PointCloud>();
    ColorCloud roi_colors;
    for (size_t i = 0; i < uv.size(); ++i)
      if (std::isfinite(uv[i].x) && std::isfinite(uv[i].y) && uv[i].x >= cx - w / 2 &&
          uv[i].x <= cx + w / 2 && uv[i].y >= cy - h / 2 && uv[i].y <= cy + h / 2)
      {
        roi->push_back(cloud[i]);
        roi_colors.push_back(colors[i]);
      }
    ColorCloud transformed_roi;
    pcl::transformPointCloud(roi_colors, transformed_roi, to_target);
    roi_all += transformed_roi;
    if (roi->size() < static_cast<size_t>(config_.min_points))
      continue;
    auto target = removeOutliers(*roi, config_.sor_mean_k, config_.sor_std_mul);
    const auto sor_size = target->size();
    target = voxelDownsample(*target, config_.voxel_size);
    auto target_colors = attachColors(*target, roi, roi_colors);
    ColorCloud transformed_target_colors;
    pcl::transformPointCloud(target_colors, transformed_target_colors, to_target);
    target_all += transformed_target_colors;  // 验收失败时也提供观测云。
    if (target->size() < static_cast<size_t>(config_.min_points))
      continue;
    pcl::transformPointCloud(*target, *target, to_target);
    std::optional<Eigen::Matrix4f> initial;
    if (config_.use_prev_init && index < last_transforms_.size())
      initial = last_transforms_[index];
    // 源云始终是 CAD 几何；不把 energy_model 当成一个待 TF 标定的实体。
    const auto result = registerClouds(source_, target, config_, initial);
    if (log)
      RCLCPP_INFO(get_logger(),
                  "ROI[%zu]: %zu -> SOR %zu -> voxel %zu, prev=%d, converged=%d, "
                  "score=%.6fm², metric=%.6f, accepted=%d",
                  index, roi->size(), sor_size, target->size(), initial.has_value(),
                  result.converged, result.fitness_score, result.fitness_metric, result.accepted);
    if (!result.accepted)
      continue;
    const auto raw_pose = poseFromTransform(result.transform);
    raw_poses.poses.push_back(poseFromTransform(result.transform * ply_to_energy_));
    candidates.push_back({index, zAxisPose(raw_pose)});
    PointCloud aligned;
    pcl::transformPointCloud(*source_, aligned, result.transform);
    raw_aligned_all += aligned;
    if (index >= last_transforms_.size())
      last_transforms_.resize(index + 1, Eigen::Matrix4f::Identity());
    last_transforms_[index] = result.transform;
  }
  constraints_->apply(candidates);
  for (const auto& candidate : candidates)
  {
    const auto energy_pose =
        poseFromTransform(transformFromPose(candidate.pose) * ply_to_energy_);
    poses.poses.push_back(energy_pose);
    PointCloud aligned;
    pcl::transformPointCloud(*source_, aligned, transformFromPose(candidate.pose));
    aligned_all += aligned;
    if (tf_broadcaster_)
    {
      geometry_msgs::msg::TransformStamped transform;
      transform.header = header;
      transform.child_frame_id = "object_" + std::to_string(candidate.detection_idx);
      transform.transform.translation.x = energy_pose.position.x;
      transform.transform.translation.y = energy_pose.position.y;
      transform.transform.translation.z = energy_pose.position.z;
      transform.transform.rotation = energy_pose.orientation;
      tf_broadcaster_->sendTransform(transform);
    }
  }
  roi_pub_->publish(cloudMessage(roi_all, header));
  target_pub_->publish(cloudMessage(target_all, header));
  raw_aligned_pub_->publish(cloudMessage(raw_aligned_all, header));
  aligned_pub_->publish(cloudMessage(aligned_all, header));
  raw_pose_pub_->publish(raw_poses);
  pose_pub_->publish(poses);
  output_active_ = !roi_all.empty() || !target_all.empty() || !poses.poses.empty();
}
}  // namespace icp_registration
