#include "stages.hpp"
#include <moveit/task_constructor/trajectory_execution_info.h>

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage6(const MotionStages &motion)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_6: home");
  auto home = std::make_unique<mtc::stages::MoveTo>("return home", motion.pipeline);
  home->setGroup(motion.settings.group);
  home->setGoal("home");
  home->setTimeout(motion.settings.planning_time);
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"arm_controller"};
  home->setProperty("trajectory_execution_info", info);
  stage->insert(std::move(home));
  stage->insert(gripperMove(motion, true));
  return stage;
}
} // namespace engineer_mtc
