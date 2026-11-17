#include "engineer_mtc/pick_stages.hpp"
namespace engineer_mtc
{

std::unique_ptr<mtc::SerialContainer> createStage1(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_1: pregrasp");

  //解除矿和夹爪的解触限制
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
