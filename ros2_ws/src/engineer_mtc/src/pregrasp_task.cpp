#include "engineer_mtc/pregrasp_task.hpp"
#include <algorithm>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/vector3_stamped.hpp>
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>

namespace engineer_mtc
{
namespace mtc = moveit::task_constructor;

// 返回 Task
std::unique_ptr<mtc::Task> createPregraspTask(const rclcpp::Node::SharedPtr& node,
                                           const TaskSettings& settings,
                                           const GraspTargets& targets)
{
  auto task = std::make_unique<mtc::Task>("engineer_mtc");
  task->stages()->setName("energy: pregrasp");
  task->loadRobotModel(node);
  if (!task->getRobotModel()->hasLinkModel(settings.tip) ||
      !task->getRobotModel()->hasJointModelGroup(settings.group))
    throw std::invalid_argument("规划末端或规划组不在机器人模型中");
  task->setProperty("group", settings.group);
  task->setProperty("ik_frame", settings.tip);
  task->setTimeout(settings.planning_time);

  // Stage 1：读真实的规划场景快照；仅指读当前模拟关节状态，不产生运动。
  auto current = std::make_unique<mtc::stages::CurrentState>("current state");
  auto valid_start = std::make_unique<mtc::stages::PredicateFilter>(
      "check current state", std::move(current));
  valid_start->setPredicate([](const mtc::SolutionBase& solution, std::string& comment) {
    const auto& scene = solution.end()->scene();
    if (!scene->getCurrentState().satisfiesBounds() || scene->isStateColliding())
    {
      comment = "当前状态超限或存在碰撞";
      return false;
    }
    return true;
  });
  auto* monitored_start = valid_start.get();
  task->add(std::move(valid_start));

  // Stage 2：连接当前状态与后面生成的预抓取关节状态。
  auto pipeline = std::make_shared<mtc::solvers::PipelinePlanner>(node, settings.pipeline);
  pipeline->setPlannerId(settings.planner_id);
  pipeline->setMaxVelocityScalingFactor(settings.velocity_scale);
  pipeline->setMaxAccelerationScalingFactor(settings.acceleration_scale);
  auto connect = std::make_unique<mtc::stages::Connect>(
      "reach pregrasp", mtc::stages::Connect::GroupPlannerVector{{settings.group, pipeline}});
  connect->setTimeout(settings.planning_time);
  task->add(std::move(connect));

  // 可选 Stage 3：从抓取候选反向求预抓取状态；返回的轨迹仍是正向接近。
  if (settings.include_approach)
  {
    auto cartesian = std::make_shared<mtc::solvers::CartesianPath>();
    cartesian->setStepSize(settings.cartesian_step);
    cartesian->setMaxVelocityScalingFactor(settings.velocity_scale);
    cartesian->setMaxAccelerationScalingFactor(settings.acceleration_scale);
    auto approach = std::make_unique<mtc::stages::MoveRelative>("approach object", cartesian);
    approach->restrictDirection(mtc::PropagatingEitherWay::BACKWARD);
    approach->setGroup(settings.group);
    approach->setIKFrame(settings.tip_to_tcp, settings.tip);
    approach->setMinMaxDistance(settings.distance, settings.distance);
    geometry_msgs::msg::Vector3Stamped direction;
    direction.header.frame_id = settings.base_frame;
    const Eigen::Vector3d axis =
        targets.grasp_tcp_base.linear() * settings.approach_axis.normalized();
    direction.vector.x = axis.x();
    direction.vector.y = axis.y();
    direction.vector.z = axis.z();
    approach->setDirection(direction);
    task->add(std::move(approach));
  }
  // 生成器提供目标姿态，ComputeIK 尝试多个关节解，避免第一段的单个 IK 解
  // 偶然无法接上直线段。监视通过起点检查的场景，不更改碰撞矩阵。
  auto generator = std::make_unique<mtc::stages::GeneratePose>("target pose");
  generator->setMonitoredStage(monitored_start);
  geometry_msgs::msg::PoseStamped goal;
  goal.header.frame_id = settings.base_frame;
  goal.pose = transformPose(settings.include_approach ?
      Eigen::Isometry3d(targets.grasp_tcp_base * settings.tip_to_tcp.inverse()) :
      targets.pregrasp_tip_base);
  generator->setPose(goal);
  auto ik = std::make_unique<mtc::stages::ComputeIK>("target IK", std::move(generator));
  ik->setGroup(settings.group);
  ik->setIKFrame(settings.tip);
  ik->setMaxIKSolutions(settings.max_ik_solutions);
  ik->setMinSolutionDistance(0.1);
  ik->setTimeout(std::min(1.0, settings.planning_time * 0.2));
  ik->properties().configureInitFrom(mtc::Stage::INTERFACE, {"target_pose"});
  task->add(std::move(ik));
  return task;
}
}  // namespace engineer_mtc
