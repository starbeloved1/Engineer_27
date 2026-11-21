#pragma once
#include "../shared/motion_stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage4(const MotionStages &);
std::unique_ptr<mtc::SerialContainer> createStage5(const MotionStages &, mtc::Stage &start,
    const Eigen::Isometry3d &storage_tip, const ContactPairs &carried,
    const ContactPairs &tool_slot, const ContactPairs &gripper);
std::unique_ptr<mtc::SerialContainer> createStage6(const MotionStages &);
} // namespace engineer_mtc
