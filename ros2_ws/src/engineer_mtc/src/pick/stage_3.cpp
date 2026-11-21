#include "stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage3(const MotionStages &motion, const GraspTargets &targets)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_3: extract");
  stage->insert(relativeMove(motion, "extract along object axis",
                             targets.object_base.linear() * motion.settings.extraction_axis,
                             motion.settings.extraction_distance));
  return stage;
}
} // namespace engineer_mtc
