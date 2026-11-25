#pragma once
#include "engineer_mtc/shared/grasp_geometry.hpp"
#include "engineer_mtc/shared/task_settings.hpp"
#include <moveit/collision_detection/collision_matrix.h>
#include <moveit/task_constructor/task.h>
#include <rclcpp/rclcpp.hpp>

namespace engineer_mtc
{
/**
 * @brief 从当前持矿状态组装阶段 4–6 的存车 Task。
 * @param node 提供机器人模型和规划参数的节点。
 * @param settings 存车规划配置。
 * @param storage_tip 基座下的槽内 link_6 目标位姿。
 * @param baseline 规划前的碰撞矩阵，用于恢复临时接触许可。
 * @return 待初始化和求解的存车 Task。
 */
std::unique_ptr<moveit::task_constructor::Task>
createPlaceTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                const Eigen::Isometry3d &storage_tip,
                const collision_detection::AllowedCollisionMatrix &baseline);
} // namespace engineer_mtc
