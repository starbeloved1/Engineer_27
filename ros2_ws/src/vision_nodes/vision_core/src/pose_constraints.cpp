// 迁移自 engineer_2026/jetson target_detection；约束顺序与公式保持不变。
#include "vision_core/pose_constraints.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace vision_core
{
PoseConstraints::PoseConstraints(const ConstraintConfig& c)
    : use_hexagon_prior_(c.use_hexagon_prior),
      hexagon_vertex_count_(c.hexagon_vertex_count),
      hexagon_min_observations_(c.hexagon_min_observations),
      hexagon_nominal_radius_(c.hexagon_nominal_radius),
      hexagon_radius_tolerance_(c.hexagon_radius_tolerance),
      hexagon_snap_max_distance_(c.hexagon_snap_max_distance),
      hexagon_snap_gain_(c.hexagon_snap_gain),
      hexagon_state_smoothing_(c.hexagon_state_smoothing),
      force_outward_z_axis_(c.force_outward_z_axis),
      enforce_z_axis_angle_constraint_(c.enforce_z_axis_angle_constraint),
      z_axis_angle_constraint_gain_(c.z_axis_angle_constraint_gain),
      z_axis_angle_max_deviation_deg_(c.z_axis_angle_max_deviation_deg)
{
}

void PoseConstraints::apply(std::vector<PoseCandidate>& candidates)
{
  update_hexagon_state(candidates);
  apply_hexagon_prior(candidates);
  force_outward_z_axis(candidates);
  enforce_z_axis_angle_constraint(candidates);
}

void PoseConstraints::reset()
{
  hexagon_state_ = HexagonState{};
}

double PoseConstraints::wrap_angle(double a)
{
  while (a > M_PI)
  {
    a -= 2.0 * M_PI;
  }
  while (a < -M_PI)
  {
    a += 2.0 * M_PI;
  }
  return a;
}

double PoseConstraints::angle_distance(double a, double b)
{
  return std::abs(wrap_angle(a - b));
}

double PoseConstraints::clamp01(double v)
{
  return std::clamp(v, 0.0, 1.0);
}

double PoseConstraints::fit_phase(const std::vector<Eigen::Vector2d>& points_xy,
                                  const Eigen::Vector2d& center_xy, int vertex_count,
                                  double initial_phase) const
{
  if (points_xy.empty() || vertex_count < 3)
  {
    return initial_phase;
  }

  auto phase_cost = [&](double phase)
  {
    double cost = 0.0;
    for (const auto& pt : points_xy)
    {
      const double a = std::atan2(pt.y() - center_xy.y(), pt.x() - center_xy.x());
      double best = std::numeric_limits<double>::max();
      for (int k = 0; k < vertex_count; ++k)
      {
        const double slot_a =
            phase + static_cast<double>(k) * 2.0 * M_PI / static_cast<double>(vertex_count);
        best = std::min(best, angle_distance(a, slot_a));
      }
      cost += best * best;
    }
    return cost;
  };

  double best_phase = initial_phase;
  double best_cost = phase_cost(best_phase);
  for (int deg = 0; deg < 360; ++deg)
  {
    const double cand = static_cast<double>(deg) * M_PI / 180.0;
    const double c = phase_cost(cand);
    if (c < best_cost)
    {
      best_cost = c;
      best_phase = cand;
    }
  }

  for (int i = -20; i <= 20; ++i)
  {
    const double cand = best_phase + static_cast<double>(i) * M_PI / 1800.0;
    const double c = phase_cost(cand);
    if (c < best_cost)
    {
      best_cost = c;
      best_phase = cand;
    }
  }
  return wrap_angle(best_phase);
}

void PoseConstraints::update_hexagon_state(const std::vector<PoseCandidate>& candidates)
{
  if (!use_hexagon_prior_ || hexagon_vertex_count_ < 3 ||
      static_cast<int>(candidates.size()) < std::max(2, hexagon_min_observations_))
  {
    return;
  }

  Eigen::Vector2d mean_xy = Eigen::Vector2d::Zero();
  double mean_z = 0.0;
  std::vector<Eigen::Vector2d> points_xy;
  points_xy.reserve(candidates.size());
  for (const auto& c : candidates)
  {
    const Eigen::Vector2d p(c.pose.position.x, c.pose.position.y);
    points_xy.push_back(p);
    mean_xy += p;
    mean_z += c.pose.position.z;
  }
  mean_xy /= static_cast<double>(points_xy.size());
  mean_z /= static_cast<double>(points_xy.size());

  double radius = 0.0;
  for (const auto& p : points_xy)
  {
    radius += (p - mean_xy).norm();
  }
  radius /= static_cast<double>(points_xy.size());
  if (hexagon_nominal_radius_ > 0.0)
  {
    if (std::abs(radius - hexagon_nominal_radius_) > hexagon_radius_tolerance_)
    {
      radius = hexagon_nominal_radius_;
    }
  }

  const double seed_phase = hexagon_state_.valid ? hexagon_state_.phase : 0.0;
  double phase = fit_phase(points_xy, mean_xy, hexagon_vertex_count_, seed_phase);

  const double alpha = clamp01(hexagon_state_smoothing_);
  if (hexagon_state_.valid)
  {
    mean_xy = alpha * mean_xy + (1.0 - alpha) * hexagon_state_.center_xy;
    mean_z = alpha * mean_z + (1.0 - alpha) * hexagon_state_.center_z;
    radius = alpha * radius + (1.0 - alpha) * hexagon_state_.radius;
    phase = wrap_angle(alpha * phase + (1.0 - alpha) * hexagon_state_.phase);
  }

  hexagon_state_.valid = true;
  hexagon_state_.center_xy = mean_xy;
  hexagon_state_.center_z = mean_z;
  hexagon_state_.radius = radius;
  hexagon_state_.phase = phase;
}

void PoseConstraints::apply_hexagon_prior(std::vector<PoseCandidate>& candidates)
{
  if (!use_hexagon_prior_ || !hexagon_state_.valid || candidates.empty() ||
      hexagon_vertex_count_ < 3)
  {
    return;
  }

  std::vector<Eigen::Vector3d> slots;
  slots.reserve(static_cast<size_t>(hexagon_vertex_count_));
  for (int k = 0; k < hexagon_vertex_count_; ++k)
  {
    const double a = hexagon_state_.phase + static_cast<double>(k) * 2.0 * M_PI /
                                                static_cast<double>(hexagon_vertex_count_);
    slots.emplace_back(hexagon_state_.center_xy.x() + hexagon_state_.radius * std::cos(a),
                       hexagon_state_.center_xy.y() + hexagon_state_.radius * std::sin(a),
                       hexagon_state_.center_z);
  }

  struct Pairing
  {
    size_t cand_idx{0};
    int slot_idx{0};
    double dist{0.0};
  };
  std::vector<Pairing> pairings;
  pairings.reserve(candidates.size() * slots.size());
  for (size_t i = 0; i < candidates.size(); ++i)
  {
    Eigen::Vector3d p(candidates[i].pose.position.x, candidates[i].pose.position.y,
                      candidates[i].pose.position.z);
    for (int s = 0; s < static_cast<int>(slots.size()); ++s)
    {
      pairings.push_back(Pairing{i, s, (p - slots[static_cast<size_t>(s)]).norm()});
    }
  }
  std::sort(pairings.begin(), pairings.end(),
            [](const Pairing& a, const Pairing& b) { return a.dist < b.dist; });

  std::vector<int> assigned_slot(candidates.size(), -1);
  std::vector<bool> used_slot(slots.size(), false);
  for (const auto& p : pairings)
  {
    if (assigned_slot[p.cand_idx] >= 0 || used_slot[static_cast<size_t>(p.slot_idx)])
    {
      continue;
    }
    assigned_slot[p.cand_idx] = p.slot_idx;
    used_slot[static_cast<size_t>(p.slot_idx)] = true;
  }

  const double snap_gain = clamp01(hexagon_snap_gain_);
  for (size_t i = 0; i < candidates.size(); ++i)
  {
    const int slot_idx = assigned_slot[i];
    if (slot_idx < 0)
    {
      continue;
    }
    const auto& slot = slots[static_cast<size_t>(slot_idx)];
    Eigen::Vector3d p(candidates[i].pose.position.x, candidates[i].pose.position.y,
                      candidates[i].pose.position.z);
    const double dist = (p - slot).norm();
    if (hexagon_snap_max_distance_ > 0.0 && dist > hexagon_snap_max_distance_)
    {
      continue;
    }

    const Eigen::Vector3d blended = (1.0 - snap_gain) * p + snap_gain * slot;
    candidates[i].pose.position.x = blended.x();
    candidates[i].pose.position.y = blended.y();
    candidates[i].pose.position.z = blended.z();

    Eigen::Vector3f outward(static_cast<float>(slot.x() - hexagon_state_.center_xy.x()),
                            static_cast<float>(slot.y() - hexagon_state_.center_xy.y()), 0.0f);
    if (outward.norm() > 1e-6f)
    {
      outward.normalize();
      const Eigen::Quaternionf q_out =
          Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(), outward);
      candidates[i].pose.orientation.x = q_out.x();
      candidates[i].pose.orientation.y = q_out.y();
      candidates[i].pose.orientation.z = q_out.z();
      candidates[i].pose.orientation.w = q_out.w();
    }
  }
}

void PoseConstraints::force_outward_z_axis(std::vector<PoseCandidate>& candidates)
{
  if (!force_outward_z_axis_ || candidates.empty())
  {
    return;
  }

  Eigen::Vector2d center_xy = Eigen::Vector2d::Zero();
  if (hexagon_state_.valid)
  {
    center_xy = hexagon_state_.center_xy;
  }
  else
  {
    for (const auto& c : candidates)
    {
      center_xy.x() += c.pose.position.x;
      center_xy.y() += c.pose.position.y;
    }
    center_xy /= static_cast<double>(candidates.size());
  }

  for (auto& c : candidates)
  {
    Eigen::Vector3f outward(static_cast<float>(c.pose.position.x - center_xy.x()),
                            static_cast<float>(c.pose.position.y - center_xy.y()), 0.0f);
    if (outward.norm() <= 1e-6f)
    {
      continue;
    }
    outward.normalize();
    const Eigen::Quaternionf q_out =
        Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(), outward);
    c.pose.orientation.x = q_out.x();
    c.pose.orientation.y = q_out.y();
    c.pose.orientation.z = q_out.z();
    c.pose.orientation.w = q_out.w();
  }
}

void PoseConstraints::enforce_z_axis_angle_constraint(std::vector<PoseCandidate>& candidates)
{
  if (!enforce_z_axis_angle_constraint_ || candidates.size() < 2 || hexagon_vertex_count_ < 3)
  {
    return;
  }

  std::vector<Eigen::Vector2d> z_dirs;
  z_dirs.reserve(candidates.size());
  std::vector<double> z_angles;
  z_angles.reserve(candidates.size());
  for (const auto& c : candidates)
  {
    const Eigen::Quaternionf q(
        static_cast<float>(c.pose.orientation.w), static_cast<float>(c.pose.orientation.x),
        static_cast<float>(c.pose.orientation.y), static_cast<float>(c.pose.orientation.z));
    Eigen::Vector3f z = q.toRotationMatrix().col(2);
    Eigen::Vector2d d(z.x(), z.y());
    if (d.norm() <= 1e-6)
    {
      continue;
    }
    d.normalize();
    z_dirs.push_back(d);
    z_angles.push_back(std::atan2(d.y(), d.x()));
  }
  if (z_dirs.size() < 2)
  {
    return;
  }

  const Eigen::Vector2d center = Eigen::Vector2d::Zero();
  const double seed_phase = hexagon_state_.valid ? hexagon_state_.phase : z_angles.front();
  const double phase = fit_phase(z_dirs, center, hexagon_vertex_count_, seed_phase);
  const double step = 2.0 * M_PI / static_cast<double>(hexagon_vertex_count_);
  const double gain = clamp01(z_axis_angle_constraint_gain_);
  const double max_dev = z_axis_angle_max_deviation_deg_ > 0.0
                             ? z_axis_angle_max_deviation_deg_ * M_PI / 180.0
                             : std::numeric_limits<double>::max();

  for (auto& c : candidates)
  {
    const Eigen::Quaternionf q(
        static_cast<float>(c.pose.orientation.w), static_cast<float>(c.pose.orientation.x),
        static_cast<float>(c.pose.orientation.y), static_cast<float>(c.pose.orientation.z));
    Eigen::Vector3f z = q.toRotationMatrix().col(2);
    Eigen::Vector2d d(z.x(), z.y());
    if (d.norm() <= 1e-6)
    {
      continue;
    }
    d.normalize();
    const double current = std::atan2(d.y(), d.x());

    double best_angle = phase;
    double best_dist = std::numeric_limits<double>::max();
    for (int k = 0; k < hexagon_vertex_count_; ++k)
    {
      const double slot_angle = phase + static_cast<double>(k) * step;
      const double dist = angle_distance(current, slot_angle);
      if (dist < best_dist)
      {
        best_dist = dist;
        best_angle = slot_angle;
      }
    }

    if (best_dist > max_dev)
    {
      continue;
    }
    const double corrected = wrap_angle(current + gain * wrap_angle(best_angle - current));
    Eigen::Vector3f constrained_z(static_cast<float>(std::cos(corrected)),
                                  static_cast<float>(std::sin(corrected)), 0.0f);
    constrained_z.normalize();
    const Eigen::Quaternionf q_out =
        Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(), constrained_z);
    c.pose.orientation.x = q_out.x();
    c.pose.orientation.y = q_out.y();
    c.pose.orientation.z = q_out.z();
    c.pose.orientation.w = q_out.w();
  }
}

}  // namespace vision_core
