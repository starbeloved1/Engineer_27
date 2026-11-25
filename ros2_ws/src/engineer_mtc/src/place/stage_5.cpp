#include "engineer_mtc/place/stages.hpp"
#include <moveit/planning_scene/planning_scene.h>

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage5(const MotionStages &motion, mtc::Stage &start,
    const Eigen::Isometry3d &storage_tip, const ContactPairs &carried,
    const ContactPairs &tool_slot, const ContactPairs &gripper)
{
  const auto &settings = motion.settings;
  auto stage = std::make_unique<mtc::SerialContainer>("stage_5: store and retreat");
  stage->insert(relativeMove(motion, "lower to storage", settings.storage_axis,
                             settings.storage_distance, true));
  stage->insert(contactIK(motion, start, "storage IK", storage_tip));
  stage->insert(gripperMove(motion, false));

  auto detach = std::make_unique<mtc::stages::ModifyPlanningScene>("detach energy");
  detach->restrictDirection(mtc::PropagatingEitherWay::FORWARD);
  detach->detachObject(settings.object_id, settings.tip);
  detach->setCallback([gripper, carried, id = settings.object_id](
                          const planning_scene::PlanningScenePtr &scene, const mtc::PropertyMap &)
  {
    auto &matrix = scene->getAllowedCollisionMatrixNonConst();
    carried.restore(matrix);
    // 放下的矿仍与车体接触，退爪时只保留必要的接触对。
    matrix.setEntry(id, scene->getRobotModel()->getRootLinkName(), true);
    gripper.allow(matrix);
  });
  stage->insert(std::move(detach));
  stage->insert(relativeMove(motion, "retreat empty gripper",
                             -storage_tip.linear() * settings.tip_to_tcp.linear() * settings.approach_axis,
                             settings.retreat_distance));

  auto store = std::make_unique<mtc::stages::ModifyPlanningScene>("restore release contact");
  store->restrictDirection(mtc::PropagatingEitherWay::FORWARD);
  store->setCallback([carried, tool_slot, id = settings.object_id](const planning_scene::PlanningScenePtr &scene,
                                                       const mtc::PropertyMap &)
  {
    auto &matrix = scene->getAllowedCollisionMatrixNonConst();
    carried.restore(matrix);
    tool_slot.restore(matrix);
    moveit_msgs::msg::CollisionObject stored;
    if (!scene->getCollisionObjectMsg(stored, id))
      throw std::runtime_error("退爪后场景缺少已经放置的矿");
    // Keep the placed geometry for return and future plans. Vision reuses the active ID.
    stored.id = storedObjectId(id);
    if (scene->getWorld()->hasObject(stored.id))
      throw std::runtime_error("存矿位已占用");
    moveit_msgs::msg::CollisionObject remove;
    remove.id = id;
    remove.operation = moveit_msgs::msg::CollisionObject::REMOVE;
    if (!scene->processCollisionObjectMsg(remove) || !scene->processCollisionObjectMsg(stored))
      throw std::runtime_error("无法保留槽内矿碰撞体");
    matrix.setEntry(stored.id, scene->getRobotModel()->getRootLinkName(), true);
  });
  stage->insert(std::move(store));
  return stage;
}
} // namespace engineer_mtc
