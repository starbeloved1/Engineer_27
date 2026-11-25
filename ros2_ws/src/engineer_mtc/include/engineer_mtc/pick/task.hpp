#pragma once
#include "engineer_mtc/shared/grasp_geometry.hpp"
#include "engineer_mtc/shared/task_settings.hpp"
#include <moveit/task_constructor/task.h>
#include <rclcpp/rclcpp.hpp>

namespace engineer_mtc
{
/**
 * @brief 组装阶段 1 的预抓取 Task。
 * @param node 提供机器人模型和规划参数的节点。
 * @param settings 取矿规划配置。
 * @param targets 基座坐标系下的取矿目标。
 * @return 待初始化和求解的预抓取 Task。
 */
std::unique_ptr<moveit::task_constructor::Task>
createPregraspTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                   const GraspTargets &targets);
/**
 * @brief 从预抓取候选末态组装阶段 2–3 的夹取与拔离 Task。
 * @param node 提供规划参数的节点。
 * @param settings 取矿规划配置。
 * @param targets 与预抓取 Task 一致的取矿目标。
 * @param start_scene 预抓取候选末态，包含关节状态、物体和碰撞许可。
 * @return 待初始化和求解的夹取与拔离 Task。
 */
std::unique_ptr<moveit::task_constructor::Task>
createPickTailTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                   const GraspTargets &targets, const planning_scene::PlanningSceneConstPtr &start_scene);
} // namespace engineer_mtc
