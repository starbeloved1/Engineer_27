#include "arm_planning/motion_planner.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <limits>
#include <moveit_msgs/msg/planning_scene_components.hpp>
#include <random>

#include "arm_planning/planning_math.hpp"

namespace arm_planning
{
using namespace std::chrono_literals;
namespace
{
template <class T>
T required(const rclcpp::Node::SharedPtr& node, const std::string& name)
{
  if (!node->has_parameter(name))
    throw std::runtime_error("Missing YAML parameter: " + name);
  return node->get_parameter(name).get_value<T>();
}
}  // namespace

PlannerSettings::PlannerSettings(const rclcpp::Node::SharedPtr& node)
{
  planning_action = required<std::string>(node, "planning_action");
  if (planning_action.empty())
    throw std::runtime_error("Empty planning_action");
  scene_service = required<std::string>(node, "scene_service");
  if (scene_service.empty())
    throw std::runtime_error("Empty scene_service");
  display_topic = required<std::string>(node, "display_topic");
  if (display_topic.empty())
    throw std::runtime_error("Empty display_topic");
  planning_group = required<std::string>(node, "planning_group");
  if (planning_group.empty())
    throw std::runtime_error("Empty planning_group");
  end_effector_link = required<std::string>(node, "end_effector_link");
  if (end_effector_link.empty())
    throw std::runtime_error("Empty end_effector_link");
  planning_pipeline = required<std::string>(node, "planning_pipeline");
  if (planning_pipeline.empty())
    throw std::runtime_error("Empty planning_pipeline");
  planner_id = required<std::string>(node, "planner_id");
  if (planner_id.empty())
    throw std::runtime_error("Empty planner_id");
  reference_frames = required<std::vector<std::string>>(node, "reference_frames");
  validation_step = required<double>(node, "validation_step_rad");
  ik_timeout = required<double>(node, "ik_timeout");
  joint_tolerance = required<double>(node, "joint_tolerance");
  default_velocity = required<double>(node, "default_velocity_scaling");
  default_acceleration = required<double>(node, "default_acceleration_scaling");
  default_planning_time = required<double>(node, "default_planning_time");
  ik_attempts = required<int>(node, "ik_attempts");
  auto positive = [](double v) { return std::isfinite(v) && v > 0; };
  if (reference_frames.empty() || !positive(validation_step) || validation_step > 0.05 ||
      ik_attempts < 1 || ik_attempts > 64 || !positive(ik_timeout) || !positive(joint_tolerance) ||
      !positive(default_velocity) || default_velocity > 1 || !positive(default_acceleration) ||
      default_acceleration > 1 || !positive(default_planning_time) || default_planning_time > 30)
    throw std::runtime_error("Invalid planner YAML parameters");
}

MotionPlanner::MotionPlanner(const rclcpp::Node::SharedPtr& node, const PlannerSettings& settings)
    : settings_(settings)
{
  move_group_ = std::make_unique<moveit::planning_interface::MoveGroupInterface>(
      node, settings_.planning_group);
  group_ = move_group_->getRobotModel()->getJointModelGroup(settings_.planning_group);
  if (!group_ || group_->getVariableCount() != 6)
    throw std::runtime_error("Planning group must contain six variables");
  const auto& names = group_->getVariableNames();
  for (size_t i = 0; i < names.size(); ++i)
    if (names[i] != "link_" + std::to_string(i + 1) + "_joint")
      throw std::runtime_error("Unexpected joint order");
  if (!move_group_->getRobotModel()->hasLinkModel(settings_.end_effector_link))
    throw std::runtime_error("Unknown end_effector_link");
  move_group_->setEndEffectorLink(settings_.end_effector_link);
  move_group_->setPlanningPipelineId(settings_.planning_pipeline);
  move_group_->setPlannerId(settings_.planner_id);
  move_group_->setGoalJointTolerance(settings_.joint_tolerance);
  scene_client_ = node->create_client<moveit_msgs::srv::GetPlanningScene>(settings_.scene_service);
  display_ =
      node->create_publisher<moveit_msgs::msg::DisplayTrajectory>(settings_.display_topic, 10);
}

void MotionPlanner::stop()
{
  move_group_->stop();
}

// 请求格式检查，关节目标和位姿目标分别检查，碰撞检查在后续场景中完成
bool MotionPlanner::validGoal(const Action::Goal& goal) const
{
  auto scaling = [](double v) { return std::isfinite(v) && v >= 0.0 && v <= 1.0; };
  if (!scaling(goal.velocity_scaling) || !scaling(goal.acceleration_scaling) ||
      !std::isfinite(goal.planning_time) || goal.planning_time < 0.0 || goal.planning_time > 30.0)
    return false;
  if (goal.target_type == Action::Goal::JOINT)
    return std::all_of(goal.joint_positions.begin(), goal.joint_positions.end(),
                       [](double q) { return std::isfinite(q); });
  if (goal.target_type != Action::Goal::POSE)
    return false;
  const auto& p = goal.target_pose.pose;
  const double values[] = {p.position.x,    p.position.y,    p.position.z,   p.orientation.x,
                           p.orientation.y, p.orientation.z, p.orientation.w};
  if (!std::all_of(std::begin(values), std::end(values), [](double v) { return std::isfinite(v); }))
    return false;
  const double norm =
      std::sqrt(p.orientation.x * p.orientation.x + p.orientation.y * p.orientation.y +
                p.orientation.z * p.orientation.z + p.orientation.w * p.orientation.w);
  return std::abs(norm - 1.0) < 0.01 &&
         (std::find(settings_.reference_frames.begin(), settings_.reference_frames.end(),
                    goal.target_pose.header.frame_id) != settings_.reference_frames.end());
}

// 通过 Service 读取 MoveIt 当前场景，返回本进程可用于碰撞检测的快照
std::shared_ptr<Scene> MotionPlanner::snapshot()
{
  if (!scene_client_->wait_for_service(3s))
    throw PlanningError(Codes::COMMUNICATION_FAILURE, "Planning scene service unavailable");
  auto request = std::make_shared<moveit_msgs::srv::GetPlanningScene::Request>();
  using C = moveit_msgs::msg::PlanningSceneComponents;
  request->components.components = C::SCENE_SETTINGS | C::ROBOT_STATE |
                                   C::ROBOT_STATE_ATTACHED_OBJECTS | C::WORLD_OBJECT_GEOMETRY |
                                   C::OCTOMAP | C::TRANSFORMS | C::ALLOWED_COLLISION_MATRIX |
                                   C::LINK_PADDING_AND_SCALING;
  auto future = scene_client_->async_send_request(request);
  if (future.wait_for(3s) != std::future_status::ready)
  {
    scene_client_->remove_pending_request(future);
    throw PlanningError(Codes::TIMED_OUT, "Planning scene request timed out");
  }
  auto scene = std::make_shared<Scene>(move_group_->getRobotModel());
  if (!scene->setPlanningSceneMsg(future.get()->scene))
    throw PlanningError(Codes::COLLISION_CHECKING_UNAVAILABLE, "Failed to read planning scene");
  return scene;
}

// 同时检查模型关节边界、自碰撞和环境障碍物，返回首个干涉部件对
std::string MotionPlanner::collision(const std::shared_ptr<Scene>& scene, const State& state)
{
  if (!state.satisfiesBounds(group_))
    return "Joint bounds violated";
  collision_detection::CollisionRequest request;
  request.group_name = settings_.planning_group;
  request.contacts = true;
  request.max_contacts = 4;
  collision_detection::CollisionResult result;
  scene->checkCollision(request, result, state);
  if (!result.collision)
    return "";
  if (result.contacts.empty())
    return "Collision detected";
  const auto& pair = result.contacts.begin()->first;
  return pair.first + " intersects " + pair.second;
}

std::vector<double> MotionPlanner::positions(const State& state)
{
  std::vector<double> q;
  state.copyJointGroupPositions(group_, q);
  return q;
}

// 将连续关节的等价角映射到靠近当前角度的一圈
State MotionPlanner::nearStart(const State& target, const State& start)
{
  State candidate(target);
  auto q = positions(candidate);
  const auto current = positions(start);
  for (size_t i = 0; i < q.size(); ++i)
  {
    // 只有连续关节可跨圈等价，保留未来有界关节的真实角度约束
    const auto* joint =
        move_group_->getRobotModel()->getJointOfVariable(group_->getVariableNames()[i]);
    if (joint->getType() == moveit::core::JointModel::REVOLUTE &&
        !joint->getVariableBounds()[0].position_bounded_)
      q[i] = wrapNear(q[i], current[i]);
  }
  candidate.setJointGroupPositions(group_, q);
  candidate.update();
  return candidate;
}

// 多种子 KDL 求解 → 碰撞筛选 → 去重 → 选择关节变化平方和最小的候选
State MotionPlanner::poseTarget(const Action::Goal& goal, const CheckCanceled& check_canceled,
                                const State& start, const std::shared_ptr<Scene>& scene,
                                Action::Result& result)
{
  const auto& message = goal.target_pose;
  if (!scene->knowsFrameTransform(message.header.frame_id))
    throw PlanningError(Codes::FRAME_TRANSFORM_FAILURE, "Unknown target frame");
  const auto& p = message.pose;
  Eigen::Quaterniond rotation(p.orientation.w, p.orientation.x, p.orientation.y, p.orientation.z);
  Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
  pose.linear() = rotation.normalized().toRotationMatrix();
  pose.translation() = Eigen::Vector3d(p.position.x, p.position.y, p.position.z);
  pose = scene->getFrameTransform(message.header.frame_id) * pose;
  const auto current = positions(start);
  std::mt19937 rng(42);
  std::uniform_real_distribution<double> perturb(-3.141592653589793, 3.141592653589793);
  double best_score = std::numeric_limits<double>::infinity();
  State best(start);
  std::vector<std::vector<double>> accepted;
  auto validity = [this, &scene](State* state, const moveit::core::JointModelGroup* group,
                                 const double* solution)
  {
    state->setJointGroupPositions(group, solution);
    state->update();
    return collision(scene, *state).empty();
  };
  for (int attempt = 0; attempt < settings_.ik_attempts; ++attempt)
  {
    check_canceled();
    State candidate(start);
    if (attempt)
    {
      auto seed = current;
      for (auto& q : seed) q += perturb(rng);
      candidate.setJointGroupPositions(group_, seed);
      candidate.enforceBounds(group_);
    }
    if (!candidate.setFromIK(group_, pose, settings_.end_effector_link, settings_.ik_timeout,
                             validity))
      continue;
    candidate = nearStart(candidate, start);
    if (!collision(scene, candidate).empty())
      continue;
    const auto q = positions(candidate);
    bool duplicate = false;
    for (const auto& previous : accepted)
    {
      const double d = squaredJointDistance(q, previous);
      if (d < 1e-8)
        duplicate = true;
    }
    if (duplicate)
      continue;
    accepted.push_back(q);
    const double score = squaredJointDistance(q, current);
    if (score < best_score)
    {
      best_score = score;
      best = candidate;
    }
  }
  result.ik_candidates = accepted.size();
  if (accepted.empty())
    throw PlanningError(Codes::NO_IK_SOLUTION,
                        "No collision-free IK solution found within the seed budget");
  return best;
}

// 每段轨迹按最大单轴角差细分，逐构型检查，不是连续碰撞证明
uint32_t MotionPlanner::validatePath(const CheckCanceled& check_canceled,
                                     const std::shared_ptr<Scene>& scene,
                                     const robot_trajectory::RobotTrajectory& trajectory)
{
  uint32_t checked = 0;
  State sample(move_group_->getRobotModel());
  for (size_t i = 0; i < trajectory.getWayPointCount(); ++i)
  {
    const auto& end = trajectory.getWayPoint(i);
    const auto& begin = trajectory.getWayPoint(i ? i - 1 : 0);
    const auto a = positions(begin), b = positions(end);
    const int subdivisions = interpolationSubdivisions(a, b, settings_.validation_step);
    for (int k = 0; k <= subdivisions; ++k)
    {
      check_canceled();
      begin.interpolate(end, static_cast<double>(k) / subdivisions, sample);
      sample.update();
      const auto issue = collision(scene, sample);
      if (!issue.empty())
        throw PlanningError(Codes::INVALID_MOTION_PLAN, "Trajectory validation: " + issue);
      ++checked;
      if (checked > 100000)
        throw PlanningError(Codes::INVALID_MOTION_PLAN, "Trajectory validation budget exceeded");
    }
  }
  return checked;
}

// 每次请求仅在工作线程执行此流程，通信状态由节点层管理
void MotionPlanner::plan(const Action::Goal& goal, Action::Result& result, const Feedback& feedback,
                         const CheckCanceled& check_canceled)
{
  check_canceled();
  feedback("checking_start", "Reading joint feedback and planning scene");
  // 1 读取起点与场景，禁止从干涉状态开始
  auto start = move_group_->getCurrentState(3.0);
  if (!start)
    throw PlanningError(Codes::ROBOT_STATE_STALE, "No current six-joint state available");
  start->update();
  auto scene = snapshot();
  const auto start_issue = collision(scene, *start);
  if (!start_issue.empty())
    throw PlanningError(Codes::START_STATE_IN_COLLISION, "Start state: " + start_issue);
  // 2 关节目标直接构造；位姿目标先求 IK
  State target(*start);
  if (goal.target_type == Action::Goal::JOINT)
  {
    target.setJointGroupPositions(group_, goal.joint_positions.data());
    target = nearStart(target, *start);
  }
  else
  {
    feedback("solving_ik", "Selecting a nearby collision-free IK candidate");
    target = poseTarget(goal, check_canceled, *start, scene, result);
  }
  const auto target_issue = collision(scene, target);
  if (!target_issue.empty())
    throw PlanningError(Codes::GOAL_IN_COLLISION, "Goal state: " + target_issue);
  const auto selected = positions(target);
  std::copy(selected.begin(), selected.end(), result.selected_joint_positions.begin());
  check_canceled();
  // 3 请求 move_group 用指定规划器搜索，库内部完成路径时间参数化
  move_group_->setStartState(*start);
  move_group_->setJointValueTarget(target);
  move_group_->setPlanningTime(goal.planning_time > 0 ? goal.planning_time
                                                      : settings_.default_planning_time);
  move_group_->setMaxVelocityScalingFactor(goal.velocity_scaling > 0 ? goal.velocity_scaling
                                                                     : settings_.default_velocity);
  move_group_->setMaxAccelerationScalingFactor(
      goal.acceleration_scaling > 0 ? goal.acceleration_scaling : settings_.default_acceleration);
  feedback("planning", settings_.planning_pipeline + "/" + settings_.planner_id);
  moveit::planning_interface::MoveGroupInterface::Plan plan;
  const auto status = move_group_->plan(plan);
  check_canceled();
  if (status.val != Codes::SUCCESS)
    throw PlanningError(status.val, "MoveIt could not find a path");
  result.planning_time = plan.planning_time_;
  robot_trajectory::RobotTrajectory trajectory(move_group_->getRobotModel(),
                                               settings_.planning_group);
  trajectory.setRobotTrajectoryMsg(*start, plan.trajectory_);
  trajectory.unwind(*start);
  if (!trajectory.getWayPointCount())
    throw PlanningError(Codes::INVALID_MOTION_PLAN, "Empty trajectory");
  feedback("validating", "Checking interpolated states against the latest scene");
  // 4 用最新场景复查返回轨迹
  scene = snapshot();
  result.validation_samples = validatePath(check_canceled, scene, trajectory);
  for (size_t i = 1; i < trajectory.getWayPointCount(); ++i)
    result.joint_path_length +=
        trajectory.getWayPoint(i - 1).distance(trajectory.getWayPoint(i), group_);
  trajectory.getRobotTrajectoryMsg(plan.trajectory_);
  result.trajectory = plan.trajectory_;
  // 5 发布预览，只有显式请求才执行到模拟控制器
  moveit_msgs::msg::DisplayTrajectory display;
  display.model_id = move_group_->getRobotModel()->getName();
  display.trajectory_start = plan.start_state_;
  display.trajectory.push_back(plan.trajectory_);
  display_->publish(display);
  if (goal.execute)
  {
    auto current = move_group_->getCurrentState(2.0);
    if (!current || current->distance(*start, group_) > 0.01)
      throw PlanningError(Codes::ROBOT_STATE_STALE,
                          "Robot moved after planning; submit a new request");
    check_canceled();
    feedback("executing", "Executing with the demo mock controller");
    const auto execution = move_group_->execute(plan);
    check_canceled();
    if (execution.val != Codes::SUCCESS)
      throw PlanningError(execution.val, "Trajectory execution failed");
  }
  result.success = true;
  result.error_code = Codes::SUCCESS;
  result.message = goal.execute ? "Planned, validated and executed in the mock demo"
                                : "Planned and validated; execution not requested";
}
}  // namespace arm_planning
