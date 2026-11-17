#include "engineer_mtc/pick_stages.hpp"
namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage3(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_3: extract");
  // 沿着矿身方向外移
  auto extract = relativeMove(c, "extract along object axis",
                              c.targets.object_base.linear() * c.settings.extraction_axis,
                              c.settings.extraction_distance);
  c.extracted_state = extract.get();
  stage->insert(std::move(extract));
  return stage;
}
} // namespace engineer_mtc
