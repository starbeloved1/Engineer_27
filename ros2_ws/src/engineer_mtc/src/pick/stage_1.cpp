#include "stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage1(const MotionStages &motion, mtc::Stage *&open)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_1: pregrasp");
  auto gripper = gripperMove(motion, false);
  open = gripper.get();
  stage->insert(std::move(gripper));
  stage->insert(armConnect(motion, "reach pregrasp"));
  return stage;
}
} // namespace engineer_mtc
