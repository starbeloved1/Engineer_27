#pragma once
#include "engineer_mtc/tasks.hpp"
#include <moveit/task_constructor/container.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>

namespace engineer_mtc
{
namespace mtc = moveit::task_constructor;
using CollisionMatrix = collision_detection::AllowedCollisionMatrix;

// A contact segment owns just the pairs it temporarily opens.
class ContactPairs
{
public:
  ContactPairs(const CollisionMatrix &baseline, std::string object, std::vector<std::string> others);
  void allow(CollisionMatrix &matrix) const;
  void restore(CollisionMatrix &matrix) const;

private:
  struct Entry
  {
    std::string other;
    bool present;
    collision_detection::AllowedCollision::Type type;
    collision_detection::DecideContactFn predicate;
  };
  std::string object_;
  std::vector<Entry> entries_;
};

struct MotionStages
{
  MotionStages(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings);
  const TaskSettings &settings;
  mtc::solvers::PipelinePlannerPtr pipeline;
  mtc::solvers::CartesianPathPtr cartesian;
  mtc::solvers::JointInterpolationPlannerPtr joints;
};

std::unique_ptr<mtc::Task> makeTask(const rclcpp::Node::SharedPtr &, const TaskSettings &,
                                    const std::string &name);
std::unique_ptr<mtc::stages::PredicateFilter> currentState();
std::unique_ptr<mtc::stages::MoveTo> gripperMove(const MotionStages &, bool close);
std::unique_ptr<mtc::stages::Connect> armConnect(const MotionStages &, const std::string &name);
std::unique_ptr<mtc::stages::MoveRelative> relativeMove(const MotionStages &, const std::string &,
                                                        const Eigen::Vector3d &, double,
                                                        bool backward = false);
std::unique_ptr<mtc::stages::ComputeIK> contactIK(
    const MotionStages &, mtc::Stage &monitor, const std::string &name,
    const Eigen::Isometry3d &goal, const ContactPairs &);
std::unique_ptr<mtc::stages::ModifyPlanningScene> startContact(
    const std::string &name, const ContactPairs &);

std::string storedObjectId(const std::string &active_id);
} // namespace engineer_mtc
