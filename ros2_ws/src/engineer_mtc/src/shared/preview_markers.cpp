#include "engineer_mtc/mtc_node.hpp"
#include <visualization_msgs/msg/marker.hpp>

namespace engineer_mtc
{
namespace
{
using visualization_msgs::msg::Marker;
constexpr char marker_namespace[] = "task_targets";

geometry_msgs::msg::Point point(const Eigen::Vector3d &position)
{
  geometry_msgs::msg::Point result;
  result.x = position.x();
  result.y = position.y();
  result.z = position.z();
  return result;
}

Marker marker(const std::string &frame, int id, int type)
{
  Marker result;
  result.header.frame_id = frame;
  // These targets are already expressed in the planning frame. A zero stamp
  // lets a late RViz subscriber resolve the frame with its current TF buffer.
  result.ns = marker_namespace;
  result.id = id;
  result.type = type;
  result.pose.orientation.w = 1.0;
  result.color.a = 1.0;
  return result;
}

Marker axes(const std::string &frame, int id, const Eigen::Isometry3d &pose)
{
  auto result = marker(frame, id, Marker::LINE_LIST);
  result.pose = transformPose(pose);
  result.scale.x = .003;
  for (int axis = 0; axis < 3; ++axis)
  {
    std_msgs::msg::ColorRGBA color;
    color.r = axis == 0 ? 1.0 : .12;
    color.g = axis == 1 ? 1.0 : .12;
    color.b = axis == 2 ? 1.0 : .12;
    color.a = 1.0;
    result.points.emplace_back();
    result.points.push_back(point(.035 * Eigen::Vector3d::Unit(axis)));
    result.colors.insert(result.colors.end(), 2, color);
  }
  return result;
}

Marker label(const std::string &frame, int id, const Eigen::Vector3d &position, const char *text)
{
  auto result = marker(frame, id, Marker::TEXT_VIEW_FACING);
  result.pose.position = point(position + .047 * Eigen::Vector3d::UnitZ());
  result.scale.z = .026;
  result.color.r = result.color.g = result.color.b = 1.0;
  result.text = text;
  return result;
}
} // namespace

void MtcNode::publishPreview(const Eigen::Isometry3d &start, const Eigen::Isometry3d &end, bool place)
{
  const auto &frame = settings_.base_frame;
  visualization_msgs::msg::MarkerArray preview;
  preview.markers.push_back(axes(frame, 0, start));
  preview.markers.push_back(axes(frame, 1, end));
  preview.markers.push_back(label(frame, 2, start.translation(), place ? "Pre-storage" : "Pregrasp"));
  preview.markers.push_back(label(frame, 3, end.translation(), place ? "Storage" : "Grasp"));
  auto arrow = marker(frame, 4, Marker::ARROW);
  arrow.points = {point(start.translation()), point(end.translation())};
  arrow.scale.x = .004;
  arrow.scale.y = .010;
  arrow.scale.z = .015;
  arrow.color.r = place ? .25 : 1.0;
  arrow.color.g = place ? .85 : .60;
  arrow.color.b = place ? 1.0 : .15;
  preview.markers.push_back(std::move(arrow));
  preview_pub_->publish(preview);
}

void MtcNode::clearPreview()
{
  auto clear = marker(settings_.base_frame, 0, Marker::ARROW);
  clear.action = Marker::DELETEALL;
  visualization_msgs::msg::MarkerArray preview;
  preview.markers.push_back(std::move(clear));
  preview_pub_->publish(preview);
}
} // namespace engineer_mtc
