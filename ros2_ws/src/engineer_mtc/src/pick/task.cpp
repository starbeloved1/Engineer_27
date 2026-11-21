#include "stages.hpp"
#include <moveit/planning_scene/planning_scene.h>

namespace engineer_mtc
{
std::unique_ptr<mtc::Task> createPickTask(const rclcpp::Node::SharedPtr &node,
                                        const TaskSettings &settings, const GraspTargets &targets)
{
  auto task = makeTask(node, settings, "external pick");
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
  mtc::Stage *open = nullptr;
  task->add(createStage1(motion, open));
  task->add(createStage2(motion, *open, targets));
  task->add(createStage3(motion, targets));
  return task;
}
} // namespace engineer_mtc
