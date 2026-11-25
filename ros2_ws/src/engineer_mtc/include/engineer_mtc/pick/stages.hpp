#pragma once
#include "engineer_mtc/shared/motion_stages.hpp"

namespace engineer_mtc
{
/// 组装阶段 1，开爪并到达预抓取位。
std::unique_ptr<mtc::SerialContainer> createStage1(const MotionStages &motion, const GraspTargets &targets);
/// 组装阶段 2，沿 TCP 方向接近、闭爪并在规划场景中附着矿。
std::unique_ptr<mtc::SerialContainer> createStage2(const MotionStages &motion, const GraspTargets &targets);
/// 组装阶段 3，保持夹持并沿矿轴拔离。
std::unique_ptr<mtc::SerialContainer> createStage3(const MotionStages &motion, const GraspTargets &targets);
} // namespace engineer_mtc
