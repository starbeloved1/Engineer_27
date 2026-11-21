#include "stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::Task> createPickTask(const rclcpp::Node::SharedPtr &node,
                                        const TaskSettings &settings, const GraspTargets &targets,
                                        const CollisionMatrix &baseline)
{
  auto task = makeTask(node, settings, "external pick");
  const MotionStages motion(node, settings);
  const ContactPairs contacts(baseline, settings.object_id, settings.touch_links);
  task->add(currentState());
  mtc::Stage *open = nullptr;
  task->add(createStage1(motion, open));
  task->add(createStage2(motion, *open, targets, contacts));
  task->add(createStage3(motion, targets));
  return task;
}
} // namespace engineer_mtc
