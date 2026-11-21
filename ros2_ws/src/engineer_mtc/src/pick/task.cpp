#include "stages.hpp"
#include <moveit/planning_scene/planning_scene.h>

namespace engineer_mtc
{
std::unique_ptr<mtc::Task> createPregraspTask(const rclcpp::Node::SharedPtr &node,
                                            const TaskSettings &settings, const GraspTargets &targets)
{
  auto task = makeTask(node, settings, "external pick: pregrasp");
  const MotionStages motion(node, settings);
  task->add(currentState());
  // 跟随 26 视觉取矿的碰撞许可，只修改取矿任务的局部规划场景。
  auto relax = std::make_unique<mtc::stages::ModifyPlanningScene>("allow pick collisions");
  relax->restrictDirection(mtc::PropagatingEitherWay::FORWARD);
  relax->setCallback([](const planning_scene::PlanningScenePtr &scene, const mtc::PropertyMap &)
  {
    relaxPickCollisions(scene->getAllowedCollisionMatrixNonConst(),
                        scene->getRobotModel()->getLinkModelNames(), scene->getWorld()->getObjectIds());
  });
  task->add(std::move(relax));
  task->add(createStage1(motion, targets));
  return task;
}

std::unique_ptr<mtc::Task> createPickTailTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                                           const GraspTargets &targets,
                                           const planning_scene::PlanningSceneConstPtr &start_scene)
{
  auto task = makeTask(node, settings, "external pick: grasp and extract", start_scene->getRobotModel());
  const MotionStages motion(node, settings);
  // 延续选中的预抓取末态和局部碰撞矩阵，避免重新从当前实测姿态开始规划尾段。
  task->add(std::make_unique<mtc::stages::FixedState>("pregrasp end state", start_scene->diff()));
  task->add(createStage2(motion, targets));
  task->add(createStage3(motion, targets));
  return task;
}
} // namespace engineer_mtc
