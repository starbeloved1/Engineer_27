#include "vision_nodes_pcl/registration.hpp"
#include "vision_nodes_pcl/pose_constraints.hpp"
#include <gtest/gtest.h>
#include <pcl/common/transforms.h>
#include <random>

using namespace vision_nodes_pcl;

namespace
{
PointCloud::Ptr testShape()
{
  auto cloud = std::make_shared<PointCloud>();
  std::mt19937 generator(27);
  std::uniform_real_distribution<float> random(-1, 1);
  for (int i = 0; i < 900; ++i)
    cloud->push_back(
        {0.07f * random(generator), 0.04f * random(generator), 0.1f * random(generator)});
  return cloud;
}
}  // namespace

TEST(Registration, RecoversKnownRigidTransform)
{
  auto source = testShape(), target = std::make_shared<PointCloud>();
  Eigen::Matrix4f expected = Eigen::Matrix4f::Identity();
  expected.block<3, 3>(0, 0) = (Eigen::AngleAxisf(0.03f, Eigen::Vector3f::UnitZ()) *
                                Eigen::AngleAxisf(-0.05f, Eigen::Vector3f::UnitY()))
                                   .toRotationMatrix();
  expected.block<3, 1>(0, 3) = Eigen::Vector3f(0.1f, -0.04f, 0.6f);
  pcl::transformPointCloud(*source, *target, expected);
  const auto result = registerClouds(source, target, RegistrationConfig{});
  ASSERT_TRUE(result.accepted);
  EXPECT_LT((result.transform - expected).norm(), 1e-4f);
  EXPECT_LT(result.fitness_score, 1e-10);
}

TEST(Registration, ScoreLimitZeroDisablesOnlyScoreGate)
{
  auto source = testShape(), target = std::make_shared<PointCloud>(*source);
  for (size_t i = 0; i < target->size(); ++i) (*target)[i].z += (i % 2 ? 0.002f : -0.002f);
  RegistrationConfig config;
  config.max_fitness_score = 1e-8;
  auto result = registerClouds(source, target, config);
  ASSERT_TRUE(result.converged);
  EXPECT_FALSE(result.accepted);
  EXPECT_GT(result.fitness_score, 1e-8);
  config.max_fitness_score = 0;
  EXPECT_TRUE(registerClouds(source, target, config).accepted);
  config.fitness_threshold = 1.0;
  EXPECT_FALSE(registerClouds(source, target, config).accepted);
}

TEST(Registration, RejectsNoCorrespondences)
{
  auto source = testShape(), target = std::make_shared<PointCloud>(*source);
  for (auto& point : *target) point.z += 2;
  RegistrationConfig config;
  config.centroid_init = false;
  config.max_corr_dist = 0.001;
  config.max_fitness_score = 0;
  EXPECT_FALSE(registerClouds(source, target, config).accepted);
}

TEST(Model, LoadsLegacyPlyWithScaleAndOriginalOrigin)
{
  const auto cloud = loadModel(MODEL_PATH, 0.001, 5000, 0.005);
  ASSERT_GT(cloud->size(), 80u);
  ASSERT_LE(cloud->size(), 1124u);
  Eigen::Vector3f low = Eigen::Vector3f::Constant(1e9), high = -low;
  for (const auto& point : *cloud)
  {
    low = low.cwiseMin(point.getVector3fMap());
    high = high.cwiseMax(point.getVector3fMap());
  }
  EXPECT_NEAR((high - low).x(), 0.095, 0.003);
  EXPECT_NEAR((high - low).y(), 0.095, 0.003);
  EXPECT_NEAR((high - low).z(), 0.150, 0.003);
  EXPECT_LT(low.z(), -0.065);  // PLY 原点不同于 Python 使用的 STL，不可擅自平移。
  EXPECT_GT(high.z(), 0.075);
  EXPECT_THROW(loadModel("/missing/energy.ply", 0.001, 5000, 0.005), std::runtime_error);
}

TEST(Constraints, ZAxisOutputRemovesAxialRotation)
{
  Eigen::Matrix4f full = Eigen::Matrix4f::Identity();
  full.block<3, 3>(0, 0) = (Eigen::AngleAxisf(0.3, Eigen::Vector3f::UnitY()) *
                            Eigen::AngleAxisf(0.7, Eigen::Vector3f::UnitZ()))
                               .toRotationMatrix();
  auto reduced = transformFromPose(zAxisPose(poseFromTransform(full)));
  EXPECT_LT((full.block<3, 1>(0, 2) - reduced.block<3, 1>(0, 2)).norm(), 1e-6);
  EXPECT_GT((full - reduced).norm(), 0.5);
}

TEST(Constraints, SixObjectsKeepSlotsAndPointOutward)
{
  PoseConstraints constraints(ConstraintConfig{});
  std::vector<PoseCandidate> candidates;
  for (int i = 0; i < 6; ++i)
  {
    geometry_msgs::msg::Pose pose;
    const double angle = i * 2 * M_PI / 6;
    pose.position.x = 0.4 + 0.227 * std::cos(angle);
    pose.position.y = -0.1 + 0.227 * std::sin(angle);
    pose.position.z = 0.3;
    pose.orientation.w = 1;
    candidates.push_back({static_cast<size_t>(i), pose});
  }
  const auto original = candidates;
  constraints.apply(candidates);
  for (size_t i = 0; i < candidates.size(); ++i)
  {
    const auto transform = transformFromPose(candidates[i].pose);
    Eigen::Vector3f radial(original[i].pose.position.x - 0.4, original[i].pose.position.y + 0.1, 0);
    EXPECT_GT((transform.block<3, 1>(0, 2).dot(radial.normalized())), 0.999);
    EXPECT_NEAR(candidates[i].pose.position.x, original[i].pose.position.x, 1e-6);
    EXPECT_NEAR(candidates[i].pose.position.y, original[i].pose.position.y, 1e-6);
  }
  constraints.reset();
  std::vector<PoseCandidate> single{original.front()};
  constraints.apply(single);
  EXPECT_NEAR(single.front().pose.orientation.w, 1, 1e-6);
}
