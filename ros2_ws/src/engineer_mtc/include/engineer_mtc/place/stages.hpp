#pragma once
#include "engineer_mtc/shared/motion_stages.hpp"

namespace engineer_mtc
{
/// 组装阶段 4，持矿搬运到预存矿位。
std::unique_ptr<mtc::SerialContainer> createStage4(const MotionStages &motion);
/// 组装阶段 5，直线入槽、松爪、解附着并退爪，保留槽内矿。
std::unique_ptr<mtc::SerialContainer> createStage5(const MotionStages &motion, mtc::Stage &start,
    const Eigen::Isometry3d &storage_tip, const ContactPairs &carried,
    const ContactPairs &tool_slot, const ContactPairs &gripper);
/// 组装阶段 6，机械臂回 home 并闭合空夹爪。
std::unique_ptr<mtc::SerialContainer> createStage6(const MotionStages &motion);
} // namespace engineer_mtc
