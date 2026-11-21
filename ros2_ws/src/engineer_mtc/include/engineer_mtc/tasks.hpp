#pragma once
#include "engineer_mtc/grasp_geometry.hpp"
#include <moveit/task_constructor/task.h>
#include <moveit/collision_detection/collision_matrix.h>
#include <rclcpp/rclcpp.hpp>
#include <string>

namespace engineer_mtc
{
struct TaskSettings
{
  std::string group, tip, base_frame, pipeline, planner_id;
  double planning_time, velocity_scale, acceleration_scale, cartesian_step, distance;
  bool include_approach;
  unsigned int max_ik_solutions;
  Eigen::Vector3d approach_axis;
  Eigen::Isometry3d tip_to_tcp;
  std::string gripper_group = "gripper", object_id = "energy";
  std::vector<std::string> touch_links = {"link_6"};
  Eigen::Isometry3d storage_energy = Eigen::Isometry3d::Identity();
  Eigen::Vector3d extraction_axis{.000000116949, .002550757964758, -.999996746811604};
  Eigen::Vector3d storage_axis{0.0, 0.0, -1.0};
  std::vector<std::string> storage_contact_objects;
  double extraction_distance = 0.10, storage_distance = 0.10, retreat_distance = 0.07;
};

std::unique_ptr<moveit::task_constructor::Task>
createPickTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                   const GraspTargets &targets,
               const collision_detection::AllowedCollisionMatrix &baseline);
std::unique_ptr<moveit::task_constructor::Task>
createPlaceTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                const Eigen::Isometry3d &tip_to_energy,
                const collision_detection::AllowedCollisionMatrix &baseline);
} // namespace engineer_mtc
