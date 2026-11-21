#pragma once
#include "engineer_mtc/grasp_geometry.hpp"
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <moveit_msgs/srv/get_planning_scene.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <mutex>

namespace engineer_mtc
{
class ContactPairs;

class EnergyScene
{
public:
  EnergyScene(const std::string &path, const std::string &id, const std::string &frame);
  bool put(const Eigen::Isometry3d &base_energy);
  void remove();
  bool attached();
  bool stored(const std::string &id);
  Eigen::Isometry3d attachedPose(const std::string &tip);
  void checkStorage(const std::string &stored_id, const std::vector<std::string> &contact_objects);
  moveit_msgs::msg::AllowedCollisionMatrix collisionMatrix();
  void restoreContacts(const ContactPairs &gripper, const ContactPairs &slot);

private:
  moveit_msgs::msg::AllowedCollisionMatrix readCollisionMatrix();
  rclcpp::Node::SharedPtr service_node_;
  rclcpp::Client<moveit_msgs::srv::GetPlanningScene>::SharedPtr scene_client_;
  moveit_msgs::msg::CollisionObject model_;
  moveit::planning_interface::PlanningSceneInterface scene_;
  std::mutex mutex_; // PlanningSceneInterface 内部服务节点不可被多个线程同时 spin。
};
} // namespace engineer_mtc
