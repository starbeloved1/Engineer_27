#include "stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::Task> createPlaceTask(const rclcpp::Node::SharedPtr &node,
                                         const TaskSettings &settings,
                                         const Eigen::Isometry3d &storage_tip,
                                         const CollisionMatrix &baseline)
{
  auto task = makeTask(node, settings, "place held energy");
  const MotionStages motion(node, settings);
  const ContactPairs slot(baseline, settings.object_id, settings.storage_contact_objects);
  const ContactPairs gripper(baseline, settings.object_id, settings.touch_links);
  auto current = currentState();
  auto *start = current.get();
  task->add(std::move(current));
  task->add(createStage4(motion));
  task->add(createStage5(motion, *start, storage_tip, slot, gripper));
  task->add(createStage6(motion));
  return task;
}
} // namespace engineer_mtc
