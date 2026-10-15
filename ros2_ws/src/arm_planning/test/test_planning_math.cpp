#include <gtest/gtest.h>

#include <cmath>

#include "arm_planning/planning_math.hpp"

using namespace arm_planning;
TEST(Angle, WrapsNearCurrentAcrossBranchCut)
{
  const double pi = std::acos(-1.);
  EXPECT_NEAR(wrapNear(-pi + .1, pi - .1), pi + .1, 1e-12);
  EXPECT_NEAR(wrapNear(4 * pi + .25, 0), .25, 1e-12);
  EXPECT_NEAR(wrapNear(.25, 4 * pi), 4 * pi + .25, 1e-12);
}
TEST(CandidateScore, PrefersNearbyJointsAndChecksDimensions)
{
  const std::vector<double> current{.1, -.2};
  EXPECT_LT(squaredJointDistance({.2, -.3}, current), squaredJointDistance({.1, 1.}, current));
  EXPECT_DOUBLE_EQ(squaredJointDistance(current, current), 0);
  EXPECT_THROW(squaredJointDistance({0}, current), std::invalid_argument);
}
TEST(Interpolation, BoundsEveryJointStepIncludingUnchangedMotion)
{
  const std::vector<double> a{0., -.2, 1.}, b{.037, -.2, .96};
  const int subdivisions = interpolationSubdivisions(a, b, .01);
  for (size_t i = 0; i < a.size(); ++i)
    EXPECT_LE(std::abs(b[i] - a[i]) / subdivisions, .01 + 1e-12);
  EXPECT_EQ(interpolationSubdivisions(a, a, .01), 1);
  EXPECT_THROW(interpolationSubdivisions(a, b, 0), std::invalid_argument);
}
