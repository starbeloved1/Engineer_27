#pragma once
#include "engineer_mtc/grasp_geometry.hpp"
#include <moveit/task_constructor/task.h>
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
};

std::unique_ptr<moveit::task_constructor::Task> createPregraspTask(
    const rclcpp::Node::SharedPtr& node, const TaskSettings& settings, const GraspTargets& targets);
}  // namespace engineer_mtc
