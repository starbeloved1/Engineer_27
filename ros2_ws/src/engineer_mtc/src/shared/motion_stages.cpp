#include "motion_stages.hpp"
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/task_constructor/storage.h>
#include <moveit/task_constructor/trajectory_execution_info.h>

namespace engineer_mtc
{
namespace
{
// ComputeIK and the backward Cartesian segment need the contact scene.
// Their upstream connection sees the baseline again at the contact boundary.
class ContactPose : public mtc::stages::GeneratePose
{
public:
  explicit ContactPose(const ContactPairs &contacts)
    : mtc::stages::GeneratePose("contact pose"), contacts_(contacts)
  {
  }

  void compute() override
  {
    const auto *upstream = upstream_solutions_.pop();
    auto scene = upstream->end()->scene()->diff();
    const auto target = properties().get<geometry_msgs::msg::PoseStamped>("pose");
    if (!scene->knowsFrameTransform(target.header.frame_id))
    {
      mtc::SubTrajectory failure;
      failure.markAsFailure("未知目标坐标系: " + target.header.frame_id);
      spawn(mtc::InterfaceState(scene), std::move(failure));
      return;
    }
    contacts_.allow(scene->getAllowedCollisionMatrixNonConst());
    mtc::InterfaceState state(scene);
    forwardProperties(*upstream->end(), state);
    state.properties().set("target_pose", target);
    spawn(std::move(state), 0.0);
  }

private:
  ContactPairs contacts_;
};
} // namespace

ContactPairs::ContactPairs(const CollisionMatrix &baseline, std::string object,
                           std::vector<std::string> others)
  : object_(std::move(object))
{
  for (auto &other : others)
  {
    Entry entry{std::move(other), false, collision_detection::AllowedCollision::NEVER, {}};
    entry.present = baseline.getEntry(object_, entry.other, entry.type);
    if (entry.type == collision_detection::AllowedCollision::CONDITIONAL)
      baseline.getEntry(object_, entry.other, entry.predicate);
    entries_.push_back(std::move(entry));
  }
}

void ContactPairs::allow(CollisionMatrix &matrix) const
{
  for (const auto &entry : entries_)
    matrix.setEntry(object_, entry.other, true);
}

void ContactPairs::restore(CollisionMatrix &matrix) const
{
  for (const auto &entry : entries_)
  {
    if (!entry.present)
      matrix.removeEntry(object_, entry.other);
    else if (entry.type == collision_detection::AllowedCollision::CONDITIONAL)
    {
      auto predicate = entry.predicate;
      matrix.setEntry(object_, entry.other, predicate);
    }
    else
      matrix.setEntry(object_, entry.other, entry.type == collision_detection::AllowedCollision::ALWAYS);
  }
}

MotionStages::MotionStages(const rclcpp::Node::SharedPtr &node, const TaskSettings &config)
  : settings(config), pipeline(std::make_shared<mtc::solvers::PipelinePlanner>(node, settings.pipeline)),
    cartesian(std::make_shared<mtc::solvers::CartesianPath>()),
    joints(std::make_shared<mtc::solvers::JointInterpolationPlanner>())
{
  pipeline->setPlannerId(settings.planner_id);
  pipeline->setProperty("num_planning_attempts", settings.max_ik_solutions);
  for (const auto &planner : std::vector<mtc::solvers::PlannerInterfacePtr>{pipeline, cartesian, joints})
  {
    planner->setMaxVelocityScalingFactor(settings.velocity_scale);
    planner->setMaxAccelerationScalingFactor(settings.acceleration_scale);
  }
  cartesian->setStepSize(settings.cartesian_step);
}

std::unique_ptr<mtc::Task> makeTask(const rclcpp::Node::SharedPtr &node, const TaskSettings &settings,
                                    const std::string &name)
{
  // Humble normalizes even a leading slash in the Task namespace.
  // Use the bare node name for /engineer_mtc/{description,statistics,solution}.
  auto task = std::make_unique<mtc::Task>(node->get_name());
  task->setName(name);
  task->loadRobotModel(node);
  const auto &model = task->getRobotModel();
  if (!model->hasLinkModel(settings.tip) || !model->hasJointModelGroup(settings.group) ||
      !model->hasJointModelGroup(settings.gripper_group))
    throw std::invalid_argument("规划末端、机械臂组或夹爪组不在模型中");
  for (const auto &link : settings.touch_links)
    if (!model->hasLinkModel(link))
      throw std::invalid_argument("未知接触 link: " + link);
  task->setProperty("group", settings.group);
  task->setProperty("ik_frame", settings.tip);
  task->setTimeout(settings.planning_time * 3);
  return task;
}

std::unique_ptr<mtc::stages::PredicateFilter> currentState()
{
  auto current = std::make_unique<mtc::stages::CurrentState>("current state");
  auto checked = std::make_unique<mtc::stages::PredicateFilter>("check current state", std::move(current));
  checked->setPredicate([](const mtc::SolutionBase &solution, std::string &comment)
  {
    if (solution.end()->scene()->getCurrentState().satisfiesBounds())
      return true;
    comment = "当前状态超限";
    return false;
  });
  return checked;
}

std::unique_ptr<mtc::stages::MoveTo> gripperMove(const MotionStages &motion, bool close)
{
  auto stage = std::make_unique<mtc::stages::MoveTo>(close ? "close gripper" : "open gripper", motion.joints);
  stage->setGroup(motion.settings.gripper_group);
  stage->setGoal(close ? "closed" : "open");
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"gripper_controller"};
  stage->setProperty("trajectory_execution_info", info);
  return stage;
}

std::unique_ptr<mtc::stages::Connect> armConnect(const MotionStages &motion, const std::string &name)
{
  auto stage = std::make_unique<mtc::stages::Connect>(
      name, mtc::stages::Connect::GroupPlannerVector{{motion.settings.group, motion.pipeline}});
  stage->setTimeout(motion.settings.planning_time);
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"arm_controller"};
  stage->setProperty("trajectory_execution_info", info);
  return stage;
}

std::unique_ptr<mtc::stages::MoveRelative> relativeMove(const MotionStages &motion,
                                                        const std::string &name,
                                                        const Eigen::Vector3d &direction,
                                                        double distance, bool backward)
{
  const auto &settings = motion.settings;
  auto stage = std::make_unique<mtc::stages::MoveRelative>(name, motion.cartesian);
  stage->restrictDirection(backward ? mtc::PropagatingEitherWay::BACKWARD
                                    : mtc::PropagatingEitherWay::FORWARD);
  stage->setGroup(settings.group);
  stage->setIKFrame(settings.tip_to_tcp, settings.tip);
  stage->setMinMaxDistance(distance, distance);
  geometry_msgs::msg::Vector3Stamped vector;
  vector.header.frame_id = settings.base_frame;
  const Eigen::Vector3d axis = direction.normalized();
  vector.vector.x = axis.x();
  vector.vector.y = axis.y();
  vector.vector.z = axis.z();
  stage->setDirection(vector);
  mtc::TrajectoryExecutionInfo info;
  info.controller_names = {"arm_controller"};
  stage->setProperty("trajectory_execution_info", info);
  return stage;
}

std::unique_ptr<mtc::stages::ComputeIK> contactIK(
    const MotionStages &motion, mtc::Stage &monitor, const std::string &name,
    const Eigen::Isometry3d &goal, const ContactPairs &contacts)
{
  auto pose = std::make_unique<ContactPose>(contacts);
  pose->setMonitoredStage(&monitor);
  geometry_msgs::msg::PoseStamped target;
  target.header.frame_id = motion.settings.base_frame;
  target.pose = transformPose(goal);
  pose->setPose(target);
  auto ik = std::make_unique<mtc::stages::ComputeIK>(name, std::move(pose));
  ik->setGroup(motion.settings.group);
  ik->setIKFrame(motion.settings.tip);
  ik->setMaxIKSolutions(motion.settings.max_ik_solutions);
  ik->setMinSolutionDistance(.1);
  ik->setTimeout(std::min(1., motion.settings.planning_time * .2));
  ik->properties().configureInitFrom(mtc::Stage::INTERFACE, {"target_pose"});
  return ik;
}

std::unique_ptr<mtc::stages::ModifyPlanningScene> startContact(
    const std::string &name, const ContactPairs &contacts)
{
  auto stage = std::make_unique<mtc::stages::ModifyPlanningScene>(name);
  // The approach is solved backward. Return to the baseline before
  // connecting the free-space motion, while execution enters contact here.
  stage->restrictDirection(mtc::PropagatingEitherWay::BACKWARD);
  stage->setCallback([contacts](const planning_scene::PlanningScenePtr &scene, const mtc::PropertyMap &)
  {
    contacts.restore(scene->getAllowedCollisionMatrixNonConst());
  });
  return stage;
}

std::string storedObjectId(const std::string &active_id)
{
  return active_id + "_stored";
}
} // namespace engineer_mtc
