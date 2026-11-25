#include "engineer_mtc/pick/task.hpp"
#include "engineer_mtc/pick/stages.hpp"
#include <moveit/planning_scene/planning_scene.h>

namespace engineer_mtc
{
// 组装预抓取 Task
std::unique_ptr<mtc::Task> createPregraspTask(const rclcpp::Node::SharedPtr &node,const TaskSettings &settings, const GraspTargets &targets)
{
  auto task = makeTask(node, settings, "external pick: pregrasp");
  const MotionStages motion(node, settings);
  task->add(currentState());
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

// 组装阶段2和3的Task
std::unique_ptr<mtc::Task> createPickTailTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,const GraspTargets &targets,const planning_scene::PlanningSceneConstPtr &start_scene)
{
  auto task = makeTask(node, settings, "external pick: grasp and extract", start_scene->getRobotModel());
  const MotionStages motion(node, settings);
  task->add(std::make_unique<mtc::stages::FixedState>("pregrasp end state", start_scene->diff()));
  task->add(createStage2(motion, targets));
  task->add(createStage3(motion, targets));
  return task;
}
} // namespace engineer_mtc
