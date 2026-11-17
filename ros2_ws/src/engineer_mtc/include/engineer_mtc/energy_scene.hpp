#pragma once
#include "engineer_mtc/grasp_geometry.hpp"
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <string>

namespace engineer_mtc
{
class EnergyScene
{
public:
  EnergyScene(const std::string &path, const std::string &id, const std::string &frame);
  bool put(const Eigen::Isometry3d &base_energy);
  void remove();
  bool attached();

private:
  moveit_msgs::msg::CollisionObject model_;
  moveit::planning_interface::PlanningSceneInterface scene_;
};
} // namespace engineer_mtc
