#include "engineer_mtc/pick_stages.hpp"
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/task_constructor/trajectory_execution_info.h>
namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage6(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_6: home");
  // 对齐 26 pick 的回位阶段，仍保留机械臂与底座的碰撞检查。
  {
  auto clear = std::make_unique<mtc::stages::ModifyPlanningScene>("wipe scene");
  clear->setCallback([group = c.settings.group](
      const planning_scene::PlanningScenePtr &scene, const mtc::PropertyMap &)
  {
    auto &acm = scene->getAllowedCollisionMatrixNonConst();
    for (const auto &link : scene->getRobotModel()->getLinkModelNames())
      acm.setDefaultEntry(link, true);
    protectArmBaseCollisions(scene, group);

    std::vector<const moveit::core::AttachedBody *> attached;
    scene->getCurrentState().getAttachedBodies(attached);
    for (const auto *body : attached)
    {
      moveit_msgs::msg::AttachedCollisionObject object;
      object.object.id = body->getName();
      object.link_name = body->getAttachedLinkName();
      object.object.operation = moveit_msgs::msg::CollisionObject::REMOVE;
      scene->processAttachedCollisionObjectMsg(object);
    }
    const auto ids = scene->getWorld()->getObjectIds();
    for (const auto &id : ids)
    {
      moveit_msgs::msg::CollisionObject object;
      object.id = id;
      object.operation = moveit_msgs::msg::CollisionObject::REMOVE;
      scene->processCollisionObjectMsg(object);
    }
  });
  stage->insert(std::move(clear));
  }
  // 机械臂回到home位置
  auto home = std::make_unique<mtc::stages::MoveTo>("return home", c.pipeline);
  home->setGroup(c.settings.group);
  home->setGoal("home");
  home->setTimeout(c.settings.planning_time);
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"arm_controller"};
  home->setProperty("trajectory_execution_info", info);
  stage->insert(std::move(home));
  stage->insert(gripperMove(c, true));
  return stage;
}
} // namespace engineer_mtc
