#pragma once
#include "engineer_mtc/pregrasp_task.hpp"
#include <moveit/task_constructor/container.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>

namespace engineer_mtc
{
namespace mtc = moveit::task_constructor;
struct PickContext
{
  const TaskSettings &settings;
  const GraspTargets &targets;
  mtc::solvers::PipelinePlannerPtr pipeline;
  mtc::solvers::CartesianPathPtr cartesian;
  mtc::solvers::JointInterpolationPlannerPtr joints;
  Eigen::Isometry3d storage_tip;
  mutable mtc::Stage *open_state = nullptr;
  mutable mtc::Stage *extracted_state = nullptr;
};
std::unique_ptr<mtc::SerialContainer> createStage1(const PickContext &context);
std::unique_ptr<mtc::SerialContainer> createStage2(const PickContext &context);
std::unique_ptr<mtc::SerialContainer> createStage3(const PickContext &context);
std::unique_ptr<mtc::SerialContainer> createStage4(const PickContext &context);
std::unique_ptr<mtc::SerialContainer> createStage5(const PickContext &context);
std::unique_ptr<mtc::SerialContainer> createStage6(const PickContext &context);

std::unique_ptr<mtc::stages::MoveTo> gripperMove(const PickContext &, bool close);
std::unique_ptr<mtc::stages::MoveRelative> relativeMove(const PickContext &, const std::string &,
                                                        const Eigen::Vector3d &, double,
                                                        bool backward = false);
} // namespace engineer_mtc
