#include "arm_planning/planning_math.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arm_planning
{
double wrapNear(double angle, double reference)
{
  return reference + std::atan2(std::sin(angle - reference), std::cos(angle - reference));
}

double squaredJointDistance(const std::vector<double>& a, const std::vector<double>& b)
{
  if (a.size() != b.size())
    throw std::invalid_argument("Joint vector sizes differ");
  double score = 0;
  for (size_t i = 0; i < a.size(); ++i) score += std::pow(a[i] - b[i], 2);
  return score;
}

int interpolationSubdivisions(const std::vector<double>& a, const std::vector<double>& b,
                              double step)
{
  if (a.size() != b.size() || !std::isfinite(step) || step <= 0)
    throw std::invalid_argument("Invalid interpolation inputs");
  double delta = 0;
  for (size_t i = 0; i < a.size(); ++i) delta = std::max(delta, std::abs(b[i] - a[i]));
  return std::max(1, static_cast<int>(std::ceil(delta / step)));
}
}  // namespace arm_planning
