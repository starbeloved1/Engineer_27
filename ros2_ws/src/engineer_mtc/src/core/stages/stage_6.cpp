#include "engineer_mtc/pick_stages.hpp"
#include <moveit/task_constructor/trajectory_execution_info.h>
namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage6(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_6: home");
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
