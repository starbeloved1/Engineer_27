#include "engineer_mtc/pick_stages.hpp"
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/task_constructor/trajectory_execution_info.h>

// core
namespace engineer_mtc
{

//gripperMove()：构建开爪或闭爪动作
std::unique_ptr<mtc::stages::MoveTo> gripperMove(const PickContext &c, bool close)
{
  // 选择joints规划器
  auto stage =
      std::make_unique<mtc::stages::MoveTo>(close ? "close gripper" : "open gripper", c.joints);
  stage->setGroup(c.settings.gripper_group);
  stage->setGoal(close ? "closed" : "open");
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"gripper_controller"};
  stage->setProperty("trajectory_execution_info", info);
  return stage;
}

//relativeMove()：构建沿指定方向直线移动的动作
std::unique_ptr<mtc::stages::MoveRelative> relativeMove(const PickContext &c,
                                                        const std::string &name,
                                                        const Eigen::Vector3d &direction,
                                                        double distance, 
                                                        bool backward)
{
  // 选择CartesianPath笛卡尔规划器求解
  auto stage = std::make_unique<mtc::stages::MoveRelative>(name, c.cartesian);
  stage->restrictDirection(backward ? mtc::PropagatingEitherWay::BACKWARD
                                    : mtc::PropagatingEitherWay::FORWARD);
  stage->setGroup(c.settings.group);
  stage->setIKFrame(c.settings.tip_to_tcp, c.settings.tip);
  stage->setMinMaxDistance(distance, distance);
  geometry_msgs::msg::Vector3Stamped vector;
  vector.header.frame_id = c.settings.base_frame;
  const Eigen::Vector3d axis = direction.normalized();
  vector.vector.x = axis.x();
  vector.vector.y = axis.y();
  vector.vector.z = axis.z();
  stage->setDirection(vector);
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"arm_controller"};
  stage->setProperty("trajectory_execution_info", info);
  return stage;
}

// 返回六个业务阶段组成的完整取存矿 Task
std::unique_ptr<mtc::Task> createPregraspTask(const rclcpp::Node::SharedPtr &node,const TaskSettings &s, const GraspTargets &targets)
{
  if (!s.include_approach)
    throw std::invalid_argument("完整 pick 要求 include_approach=true");
  
  // 创建task加载模型
  auto task = std::make_unique<mtc::Task>("engineer_mtc");
  {
  task->stages()->setName("energy: complete pick");
  task->loadRobotModel(node);
  }

  const auto &model = task->getRobotModel();
  if (!model->hasLinkModel(s.tip) || !model->hasJointModelGroup(s.group) ||
      !model->hasJointModelGroup(s.gripper_group))
    throw std::invalid_argument("规划末端、机械臂组或夹爪组不在模型中");
  for (const auto &link : s.touch_links)
    if (!model->hasLinkModel(link))
      throw std::invalid_argument("未知接触 link: " + link);

  // config
  {
  task->setProperty("group", s.group);
  task->setProperty("ik_frame", s.tip);
  task->setTimeout(s.planning_time * 6);
  }

  // 建立规划起点并且检查状态
  {
  auto current = std::make_unique<mtc::stages::CurrentState>("current state");
  auto start = std::make_unique<mtc::stages::PredicateFilter>("check current state", std::move(current));
  start->setPredicate(
      [](const mtc::SolutionBase &solution, std::string &comment)
      {
        const auto &scene = solution.end()->scene();
        if (!scene->getCurrentState().satisfiesBounds() || scene->isStateColliding())
        {
          comment = "当前状态超限或存在碰撞";
          return false;
        }
        return true;
      });
  task->add(std::move(start));
  }

  // 创建pipline，cartesian和joints三个规划器，初始化打包
  PickContext c{s,
                targets,
                std::make_shared<mtc::solvers::PipelinePlanner>(node, s.pipeline),
                std::make_shared<mtc::solvers::CartesianPath>(),
                std::make_shared<mtc::solvers::JointInterpolationPlanner>(),
                Eigen::Isometry3d::Identity()};
  {
  c.pipeline->setPlannerId(s.planner_id);
  c.pipeline->setProperty("num_planning_attempts", s.max_ik_solutions);
  for (const auto &planner :
       std::vector<mtc::solvers::PlannerInterfacePtr>{c.pipeline, c.cartesian, c.joints})
  {
    planner->setMaxVelocityScalingFactor(s.velocity_scale);
    planner->setMaxAccelerationScalingFactor(s.acceleration_scale);
  }
  c.cartesian->setStepSize(s.cartesian_step);
  }

  // 前置计算，存矿时位姿
  {
  const Eigen::Isometry3d grasp_tip = targets.grasp_tcp_base * s.tip_to_tcp.inverse();
  const Eigen::Isometry3d tip_energy = grasp_tip.inverse() * targets.object_base * s.object_to_energy;
  c.storage_tip = s.storage_energy * tip_energy.inverse();
  }
  
  // Task被分为6个业务阶段
  {
  task->add(createStage1(c));
  task->add(createStage2(c));
  task->add(createStage3(c));
  task->add(createStage4(c));
  task->add(createStage5(c));
  task->add(createStage6(c));
  }

  return task;
}
} // namespace engineer_mtc
