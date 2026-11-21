#include "stages.hpp"
#include <moveit/planning_scene/planning_scene.h>

namespace engineer_mtc
{
std::unique_ptr<mtc::Task> createPlaceTask(const rclcpp::Node::SharedPtr &node,
                                         const TaskSettings &settings,
                                         const Eigen::Isometry3d &storage_tip,
                                         const CollisionMatrix &baseline)
{
  auto task = makeTask(node, settings, "place held energy");
  const MotionStages motion(node, settings);
  const auto &model = *task->getRobotModel();
  const ContactPairs arm_base(baseline, model.getRootLinkName(),
                              model.getJointModelGroup(settings.group)->getLinkModelNames());
  // 26 的 claw_execute_link 包含随爪运动的矿形几何，27 用实际附着的矿表示。
  const auto carried = carriedPlaceContacts(baseline, settings, model.getLinkModelNames());
  const ContactPairs tool_slot(baseline, settings.tip, settings.storage_contact_objects);
  const ContactPairs gripper(baseline, settings.object_id, settings.touch_links);
  task->add(currentState());
  auto allow = std::make_unique<mtc::stages::ModifyPlanningScene>("allow arm-base, carried energy and slot contacts");
  allow->restrictDirection(mtc::PropagatingEitherWay::FORWARD);
  allow->setCallback([arm_base, carried, tool_slot](const planning_scene::PlanningScenePtr &scene,
                                                   const mtc::PropertyMap &)
  {
    auto &matrix = scene->getAllowedCollisionMatrixNonConst();
    arm_base.allow(matrix);
    carried.allow(matrix);
    tool_slot.allow(matrix);
  });
  auto *start = allow.get();
  task->add(std::move(allow));
  task->add(createStage4(motion));
  task->add(createStage5(motion, *start, storage_tip, carried, tool_slot, gripper));
  task->add(createStage6(motion));
  return task;
}
} // namespace engineer_mtc
