#include "engineer_mtc/pick_stages.hpp"
#include <moveit/planning_scene/planning_scene.h>
namespace engineer_mtc
{

std::unique_ptr<mtc::SerialContainer> createStage1(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_1: pregrasp");

  // 对齐 26 pick：机器人各 link 默认允许碰撞，贯穿后续完整 Task。
  // setDefaultEntry 不覆盖已有的显式碰撞对。
  {
  auto relax = std::make_unique<mtc::stages::ModifyPlanningScene>("relax collision monitoring");
  relax->setCallback([group = c.settings.group, object = c.settings.object_id](
      const planning_scene::PlanningScenePtr &scene, const mtc::PropertyMap &)
  {
    auto &acm = scene->getAllowedCollisionMatrixNonConst();
    for (const auto &link : scene->getRobotModel()->getLinkModelNames())
      acm.setDefaultEntry(link, true);
    acm.setEntry(object, scene->getRobotModel()->getJointModelGroup(group)
                            ->getLinkModelNamesWithCollisionGeometry(), true);
    protectArmBaseCollisions(scene, group);
  });
  stage->insert(std::move(relax));
  }

  // 兼容额外配置在机械臂组之外的夹爪接触 link。
  {
  auto contact = std::make_unique<mtc::stages::ModifyPlanningScene>("allow gripper contact");
  contact->allowCollisions(c.settings.object_id, c.settings.touch_links, true);
  stage->insert(std::move(contact));
  }

  // 打开夹爪
  {
  auto open = gripperMove(c, false);
  c.open_state = open.get();
  stage->insert(std::move(open));
  }

  // 连接到预抓取状态
  {
  auto connect = std::make_unique<mtc::stages::Connect>(
      "reach pregrasp", mtc::stages::Connect::GroupPlannerVector{{c.settings.group, c.pipeline}});
  connect->setTimeout(c.settings.planning_time);
  stage->insert(std::move(connect));
  }

  return stage;
}
} // namespace engineer_mtc
