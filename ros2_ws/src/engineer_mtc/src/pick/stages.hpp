#pragma once
#include "../shared/motion_stages.hpp"

namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage1(const MotionStages &, mtc::Stage *&open);
std::unique_ptr<mtc::SerialContainer> createStage2(const MotionStages &, mtc::Stage &open,
                                                  const GraspTargets &);
std::unique_ptr<mtc::SerialContainer> createStage3(const MotionStages &, const GraspTargets &);
} // namespace engineer_mtc
