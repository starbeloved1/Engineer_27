#pragma once
#include <Eigen/Dense>
#include <geometry_msgs/msg/pose.hpp>
#include <vector>

namespace vision_core
{
struct PoseCandidate
{
  size_t detection_idx{0};
  geometry_msgs::msg::Pose pose;
};

struct ConstraintConfig
{
  bool use_hexagon_prior{true};
  int hexagon_vertex_count{6}, hexagon_min_observations{3};
  double hexagon_nominal_radius{0.0}, hexagon_radius_tolerance{0.03};
  double hexagon_snap_max_distance{0.04}, hexagon_snap_gain{0.8}, hexagon_state_smoothing{0.35};
  bool force_outward_z_axis{true}, enforce_z_axis_angle_constraint{true};
  double z_axis_angle_constraint_gain{1.0}, z_axis_angle_max_deviation_deg{180.0};
};

// 六矿布局先验属于旧版业务约束，不是通用 ICP 或目标身份跟踪。
class PoseConstraints
{
 public:
  explicit PoseConstraints(const ConstraintConfig& config);
  void apply(std::vector<PoseCandidate>& candidates);
  void reset();

 private:
  struct HexagonState
  {
    bool valid{false};
    Eigen::Vector2d center_xy{Eigen::Vector2d::Zero()};
    double center_z{0.0}, radius{0.0}, phase{0.0};
  };
  static double wrap_angle(double a);
  static double angle_distance(double a, double b);
  static double clamp01(double v);
  double fit_phase(const std::vector<Eigen::Vector2d>& points_xy, const Eigen::Vector2d& center_xy,
                   int vertex_count, double initial_phase = 0.0) const;
  void update_hexagon_state(const std::vector<PoseCandidate>& candidates);
  void apply_hexagon_prior(std::vector<PoseCandidate>& candidates);
  void force_outward_z_axis(std::vector<PoseCandidate>& candidates);
  void enforce_z_axis_angle_constraint(std::vector<PoseCandidate>& candidates);
  bool use_hexagon_prior_{};
  int hexagon_vertex_count_{};
  int hexagon_min_observations_{};
  double hexagon_nominal_radius_{};
  double hexagon_radius_tolerance_{};
  double hexagon_snap_max_distance_{};
  double hexagon_snap_gain_{};
  double hexagon_state_smoothing_{};
  bool force_outward_z_axis_{};
  bool enforce_z_axis_angle_constraint_{};
  double z_axis_angle_constraint_gain_{};
  double z_axis_angle_max_deviation_deg_{};
  HexagonState hexagon_state_;
};
}  // namespace vision_core
