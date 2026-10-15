#pragma once

#include <vector>

namespace arm_planning
{
// 只包含角度与距离计算，不依赖 ROS 或 MoveIt
double wrapNear(double angle, double reference);
double squaredJointDistance(const std::vector<double>& a, const std::vector<double>& b);
int interpolationSubdivisions(const std::vector<double>& a, const std::vector<double>& b,
                              double step);
}  // namespace arm_planning
