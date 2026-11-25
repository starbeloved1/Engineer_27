#include "engineer_mtc/place/stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage4(const MotionStages &motion)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_4: transfer");
  stage->insert(armConnect(motion, "reach prestorage"));
  return stage;
}
} // namespace engineer_mtc
