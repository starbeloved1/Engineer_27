#include "engineer_mtc/pick_stages.hpp"
namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage4(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_4: transfer");
  // 规划移动到预存矿位置
  auto connect = std::make_unique<mtc::stages::Connect>(
      "reach prestorage", mtc::stages::Connect::GroupPlannerVector{{c.settings.group, c.pipeline}});
  connect->setTimeout(c.settings.planning_time);
  stage->insert(std::move(connect));
  return stage;
}
} // namespace engineer_mtc
