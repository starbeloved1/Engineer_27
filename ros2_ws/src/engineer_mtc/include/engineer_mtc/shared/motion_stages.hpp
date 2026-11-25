#pragma once
#include "engineer_mtc/shared/grasp_geometry.hpp"
#include "engineer_mtc/shared/task_settings.hpp"
#include <moveit/collision_detection/collision_matrix.h>
#include <moveit/task_constructor/container.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>
#include <moveit/task_constructor/task.h>

// 封装常用的MTC命令
namespace engineer_mtc
{
namespace mtc = moveit::task_constructor;
using CollisionMatrix = collision_detection::AllowedCollisionMatrix;

/// 保存指定物体与其他物体的原碰撞许可，以便临时放行后逐对恢复。
class ContactPairs
{
public:
  /// 创建空接触对集合。
  ContactPairs() = default;
  /// 从 baseline 记录 object 与 others 之间的碰撞许可。
  ContactPairs(const CollisionMatrix &baseline, std::string object, std::vector<std::string> others);
  /// 在给定矩阵中放行本集合的接触对。
  void allow(CollisionMatrix &matrix) const;
  /// 在给定矩阵中恢复本集合记录的原许可。
  void restore(CollisionMatrix &matrix) const;

private:
  struct Entry
  {
    std::string other;
    bool present;
    collision_detection::AllowedCollision::Type type;
    collision_detection::DecideContactFn predicate;
  };
  std::string object_;
  std::vector<Entry> entries_;
};

/// 放行给定连杆之间及其与场景物体之间的碰撞。
void relaxPickCollisions(CollisionMatrix &matrix, const std::vector<std::string> &links,const std::vector<std::string> &objects);
/// 记录持矿与机器人连杆、槽内接触物体的原碰撞许可
ContactPairs carriedPlaceContacts(const CollisionMatrix &baseline, const TaskSettings &settings,std::vector<std::string> links);

struct MotionStages
{
  /// 根据节点和配置创建 OMPL、笛卡尔和关节插值规划器
  MotionStages(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings);
  const TaskSettings &settings;
  mtc::solvers::PipelinePlannerPtr pipeline;
  mtc::solvers::CartesianPathPtr cartesian;
  mtc::solvers::JointInterpolationPlannerPtr joints;
};

/// 创建已加载机器人模型与组配置的 Task，model 为空时从节点加载
std::unique_ptr<mtc::Task> makeTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                                    const std::string &name,
                                    const moveit::core::RobotModelConstPtr &model = {});
/// 创建读取当前机器人状态并检查关节范围的阶段
std::unique_ptr<mtc::stages::PredicateFilter> currentState();
/// 创建夹爪运动阶段，close 为 true 时闭合，为 false 时打开
std::unique_ptr<mtc::stages::MoveTo> gripperMove(const MotionStages &motion, bool close);
/// 创建名为 name 的机械臂连接阶段，使用 OMPL 规划器
std::unique_ptr<mtc::stages::Connect> armConnect(const MotionStages &motion, const std::string &name);
/// 创建沿基座下 direction 移动 distance 米的直线阶段
std::unique_ptr<mtc::stages::MoveRelative> relativeMove(const MotionStages &motion, const std::string &name,
                                                        const Eigen::Vector3d &direction, double distance,
                                                        bool backward = false);
/// 基于 monitor 的场景为基座下 link_6 目标生成IK候选，pick 选择取矿候选数
std::unique_ptr<mtc::stages::ComputeIK> contactIK(const MotionStages &motion, mtc::Stage &monitor, const std::string &name,const Eigen::Isometry3d &goal, bool pick = false);

std::string storedObjectId(const std::string &active_id);
} // namespace engineer_mtc
