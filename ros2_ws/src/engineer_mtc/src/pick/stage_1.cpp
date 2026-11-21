#include "stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage1(const MotionStages &motion, const GraspTargets &targets)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_1: pregrasp");
  auto gripper = gripperMove(motion, false);
  auto *open = gripper.get();
  stage->insert(std::move(gripper));
  stage->insert(armConnect(motion, "reach pregrasp"));
  stage->insert(contactIK(motion, *open, "pregrasp IK", targets.pregrasp_tip_base, /*pick=*/true));
  return stage;
}
} // namespace engineer_mtc
