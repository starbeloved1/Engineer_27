#include "stages.hpp"
#include <moveit/planning_scene/planning_scene.h>

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage2(const MotionStages &motion, mtc::Stage &open,
                                                  const GraspTargets &targets)
{
  const auto &settings = motion.settings;
  auto stage = std::make_unique<mtc::SerialContainer>("stage_2: grasp and attach");
  stage->insert(relativeMove(motion, "approach waist",
                            targets.grasp_tcp_base.linear() * settings.approach_axis,
                            settings.distance, true));
  stage->insert(contactIK(motion, open, "grasp IK",
                         targets.grasp_tcp_base * settings.tip_to_tcp.inverse()));
  stage->insert(gripperMove(motion, true));

  auto attach = std::make_unique<mtc::stages::ModifyPlanningScene>("attach energy");
  attach->restrictDirection(mtc::PropagatingEitherWay::FORWARD);
  attach->setCallback([id = settings.object_id, tip = settings.tip, links = settings.touch_links](
                          const planning_scene::PlanningScenePtr &scene, const mtc::PropertyMap &)
  {
    moveit_msgs::msg::AttachedCollisionObject object;
    object.link_name = tip;
    object.touch_links = links;
    object.object.id = id;
    object.object.operation = moveit_msgs::msg::CollisionObject::ADD;
    if (!scene->processAttachedCollisionObjectMsg(object))
      throw std::runtime_error("无法将矿附着到末端");
  });
  stage->insert(std::move(attach));
  return stage;
}
} // namespace engineer_mtc
