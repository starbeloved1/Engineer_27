#pragma once
#include "engineer_mtc/grasp_geometry.hpp"
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <string>
#include <mutex>

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
  std::mutex mutex_; // PlanningSceneInterface 内部服务节点不可被多个线程同时 spin。
};
} // namespace engineer_mtc
