#include <gtest/gtest.h>

#include "arm_goal/goal_client_node.hpp"

using namespace arm_goal;
TEST(GoalOptions, JointOrderAndOfflineDefault)
{
  const auto options = parseGoalOptions(
      {"plan_goal", "--joints", ".1", "-.2", ".3", "0", ".5", ".6", "--output", "/tmp/plan.json"});
  EXPECT_EQ(options.goal.target_type, options.goal.JOINT);
  EXPECT_DOUBLE_EQ(options.goal.joint_positions[1], -.2);
  EXPECT_DOUBLE_EQ(options.goal.joint_positions[5], .6);
  EXPECT_FALSE(options.goal.execute);
  EXPECT_EQ(options.output, "/tmp/plan.json");
  EXPECT_DOUBLE_EQ(options.goal.planning_time, 0);
}
TEST(GoalOptions, PoseQuaternionOrderAndOverrides)
{
  const auto options = parseGoalOptions({"plan_goal",
                                         "--pose",
                                         ".1",
                                         ".2",
                                         ".3",
                                         "0",
                                         "0",
                                         "0",
                                         "1",
                                         "--frame",
                                         "world",
                                         "--execute",
                                         "--velocity-scale",
                                         ".3",
                                         "--acceleration-scale",
                                         ".4",
                                         "--planning-time",
                                         "6",
                                         "--timeout",
                                         "8"});
  EXPECT_EQ(options.goal.target_type, options.goal.POSE);
  EXPECT_EQ(options.goal.target_pose.header.frame_id, "world");
  EXPECT_DOUBLE_EQ(options.goal.target_pose.pose.orientation.w, 1);
  EXPECT_TRUE(options.goal.execute);
  EXPECT_DOUBLE_EQ(options.goal.velocity_scaling, .3);
  EXPECT_DOUBLE_EQ(options.goal.acceleration_scaling, .4);
  EXPECT_DOUBLE_EQ(options.goal.planning_time, 6);
  EXPECT_DOUBLE_EQ(options.timeout, 8);
}
TEST(GoalOptions, RejectsMalformedInputs)
{
  EXPECT_THROW(parseGoalOptions({"plan_goal"}), std::invalid_argument);
  EXPECT_THROW(parseGoalOptions({"plan_goal", "--joints", "0"}), std::invalid_argument);
  EXPECT_THROW(parseGoalOptions({"plan_goal", "--joints", "nan", "0", "0", "0", "0", "0"}),
               std::invalid_argument);
  EXPECT_THROW(parseGoalOptions({"plan_goal", "--pose", "0", "0", "0", "0", "0", "0", "0"}),
               std::invalid_argument);
  EXPECT_THROW(parseGoalOptions({"plan_goal", "--joints", "0", "0", "0", "0", "0", "0",
                                 "--velocity-scale", "2"}),
               std::invalid_argument);
  EXPECT_THROW(
      parseGoalOptions({"plan_goal", "--joints", "0", "0", "0", "0", "0", "0", "--pose", "0"}),
      std::invalid_argument);
}

TEST(GoalOptions, TopicModeDoesNotAcceptSingleGoalOptions)
{
  EXPECT_TRUE(parseGoalOptions({"plan_goal", "--listen"}).listen);
  EXPECT_THROW(parseGoalOptions({"plan_goal", "--listen", "--execute"}), std::invalid_argument);
  EXPECT_THROW(parseGoalOptions({"plan_goal", "--listen", "--joints", "0", "0", "0", "0", "0", "0"}),
               std::invalid_argument);
}
