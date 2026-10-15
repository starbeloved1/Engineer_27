#include <arm_planning/action/plan_motion.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/robot_state/conversions.h>
#include <moveit/robot_trajectory/robot_trajectory.h>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <moveit_msgs/msg/planning_scene_components.hpp>
#include <moveit_msgs/srv/get_planning_scene.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <future>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using Action = arm_planning::action::PlanMotion;
using Handle = rclcpp_action::ServerGoalHandle<Action>;
using Codes = moveit_msgs::msg::MoveItErrorCodes;
using State = moveit::core::RobotState;
using Scene = planning_scene::PlanningScene;
using namespace std::chrono_literals;

struct PlanningError : std::runtime_error
{
  int code;
  PlanningError(int value, const std::string& message) : std::runtime_error(message), code(value) {}
};

class ArmPlanner
{
public:
  explicit ArmPlanner(const rclcpp::Node::SharedPtr& node) : node_(node)
  {
    step_ = parameter("validation_step_rad", 0.01);
    ik_attempts_ = parameter("ik_attempts", 12);
    if (!(step_ > 0.0 && step_ <= 0.05) || ik_attempts_ < 1 || ik_attempts_ > 64)
      throw std::runtime_error("Invalid validation_step_rad or ik_attempts");
    move_group_ = std::make_unique<moveit::planning_interface::MoveGroupInterface>(node_, "arm_control");
    group_ = move_group_->getRobotModel()->getJointModelGroup("arm_control");
    if (!group_ || group_->getVariableCount() != 6)
      throw std::runtime_error("arm_control must contain exactly six variables");
    const auto& names = group_->getVariableNames();
    for (size_t i = 0; i < names.size(); ++i)
      if (names[i] != "link_" + std::to_string(i + 1) + "_joint")
        throw std::runtime_error("Unexpected joint order");
    move_group_->setEndEffectorLink("link_6");
    move_group_->setPlanningPipelineId("ompl");
    move_group_->setPlannerId("RRTConnectkConfigDefault");
    move_group_->setGoalJointTolerance(1e-4);
    scene_client_ = node_->create_client<moveit_msgs::srv::GetPlanningScene>("/get_planning_scene");
    display_ = node_->create_publisher<moveit_msgs::msg::DisplayTrajectory>("/display_planned_path", 10);
    server_ = rclcpp_action::create_server<Action>(node_, "/arm_planning/plan_motion",
      [this](const rclcpp_action::GoalUUID&, const std::shared_ptr<const Action::Goal> goal) {
        if (!validGoal(*goal)) return rclcpp_action::GoalResponse::REJECT;
        bool expected = false;
        if (!busy_.compare_exchange_strong(expected, true)) return rclcpp_action::GoalResponse::REJECT;
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
      },
      [this](const std::shared_ptr<Handle>) {
        cancel_requested_.store(true);
        return rclcpp_action::CancelResponse::ACCEPT;
      },
      [this](const std::shared_ptr<Handle> handle) {
        if (worker_.joinable()) worker_.join();
        worker_ = std::thread([this, handle] { run(handle); });
      });
    // This timer shares the action server's mutually exclusive callback group.
    // Stop only after the cancellation callback has returned and the ROS action
    // has actually entered CANCELING, avoiding a result/cancel transition race.
    cancel_timer_ = node_->create_wall_timer(20ms, [this] {
      if (cancel_requested_.exchange(false)) move_group_->stop();
    });
    RCLCPP_INFO(node_->get_logger(), "Ready: /arm_planning/plan_motion; tip=link_6; offline mock execution");
  }

  ~ArmPlanner()
  {
    if (worker_.joinable()) {
      if (rclcpp::ok()) move_group_->stop();
      worker_.join();
    }
  }

private:
  template<class T> T parameter(const std::string& name, T fallback)
  {
    if (!node_->has_parameter(name)) return node_->declare_parameter<T>(name, fallback);
    return node_->get_parameter(name).get_value<T>();
  }

  static bool validGoal(const Action::Goal& goal)
  {
    auto scaling = [](double v) { return std::isfinite(v) && v >= 0.0 && v <= 1.0; };
    if (!scaling(goal.velocity_scaling) || !scaling(goal.acceleration_scaling) ||
        !std::isfinite(goal.planning_time) || goal.planning_time < 0.0 || goal.planning_time > 30.0) return false;
    if (goal.target_type == Action::Goal::JOINT)
      return std::all_of(goal.joint_positions.begin(), goal.joint_positions.end(), [](double q) { return std::isfinite(q); });
    if (goal.target_type != Action::Goal::POSE) return false;
    const auto& p = goal.target_pose.pose;
    const double values[] = {p.position.x, p.position.y, p.position.z,
                            p.orientation.x, p.orientation.y, p.orientation.z, p.orientation.w};
    if (!std::all_of(std::begin(values), std::end(values), [](double v) { return std::isfinite(v); })) return false;
    const double norm = std::sqrt(p.orientation.x * p.orientation.x + p.orientation.y * p.orientation.y +
                                  p.orientation.z * p.orientation.z + p.orientation.w * p.orientation.w);
    return std::abs(norm - 1.0) < 0.01 &&
      (goal.target_pose.header.frame_id == "world" || goal.target_pose.header.frame_id == "base_link");
  }

  void feedback(const std::shared_ptr<Handle>& handle, const std::string& stage, const std::string& detail)
  {
    auto message = std::make_shared<Action::Feedback>();
    message->stage = stage;
    message->detail = detail;
    handle->publish_feedback(message);
  }

  static void checkCanceled(const std::shared_ptr<Handle>& handle)
  {
    if (handle->is_canceling() || !rclcpp::ok()) throw PlanningError(Codes::PREEMPTED, "Request canceled");
  }

  std::shared_ptr<Scene> snapshot()
  {
    if (!scene_client_->wait_for_service(3s)) throw PlanningError(Codes::COMMUNICATION_FAILURE, "Planning scene service unavailable");
    auto request = std::make_shared<moveit_msgs::srv::GetPlanningScene::Request>();
    using C = moveit_msgs::msg::PlanningSceneComponents;
    request->components.components = C::SCENE_SETTINGS | C::ROBOT_STATE | C::ROBOT_STATE_ATTACHED_OBJECTS |
      C::WORLD_OBJECT_GEOMETRY | C::OCTOMAP | C::TRANSFORMS | C::ALLOWED_COLLISION_MATRIX | C::LINK_PADDING_AND_SCALING;
    auto future = scene_client_->async_send_request(request);
    if (future.wait_for(3s) != std::future_status::ready) {
      scene_client_->remove_pending_request(future);
      throw PlanningError(Codes::TIMED_OUT, "Planning scene request timed out");
    }
    auto scene = std::make_shared<Scene>(move_group_->getRobotModel());
    if (!scene->setPlanningSceneMsg(future.get()->scene))
      throw PlanningError(Codes::COLLISION_CHECKING_UNAVAILABLE, "Failed to read planning scene");
    return scene;
  }

  std::string collision(const std::shared_ptr<Scene>& scene, const State& state)
  {
    if (!state.satisfiesBounds(group_)) return "Joint bounds violated";
    collision_detection::CollisionRequest request;
    request.group_name = "arm_control";
    request.contacts = true;
    request.max_contacts = 4;
    collision_detection::CollisionResult result;
    scene->checkCollision(request, result, state);
    if (!result.collision) return "";
    if (result.contacts.empty()) return "Collision detected";
    const auto& pair = result.contacts.begin()->first;
    return pair.first + " intersects " + pair.second;
  }

  std::vector<double> positions(const State& state)
  {
    std::vector<double> q;
    state.copyJointGroupPositions(group_, q);
    return q;
  }

  State nearStart(const State& target, const State& start)
  {
    State candidate(target);
    auto q = positions(candidate);
    const auto current = positions(start);
    for (size_t i = 0; i < q.size(); ++i) {
      // Preserve future revolute bounds; only continuous joints can wrap.
      const auto* joint = group_->getJointModels()[i];
      if (joint->getType() == moveit::core::JointModel::REVOLUTE && !joint->getVariableBounds()[0].position_bounded_)
        q[i] = current[i] + std::atan2(std::sin(q[i] - current[i]), std::cos(q[i] - current[i]));
    }
    candidate.setJointGroupPositions(group_, q);
    candidate.update();
    return candidate;
  }

  State poseTarget(const std::shared_ptr<Handle>& handle, const State& start,
                   const std::shared_ptr<Scene>& scene, Action::Result& result)
  {
    const auto& message = handle->get_goal()->target_pose;
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
    auto validity = [this, &scene](State* state, const moveit::core::JointModelGroup* group, const double* solution) {
      state->setJointGroupPositions(group, solution);
      state->update();
      return collision(scene, *state).empty();
    };
    for (int attempt = 0; attempt < ik_attempts_; ++attempt) {
      checkCanceled(handle);
      State candidate(start);
      if (attempt) {
        auto seed = current;
        for (auto& q : seed) q += perturb(rng);
        candidate.setJointGroupPositions(group_, seed);
        candidate.enforceBounds(group_);
      }
      if (!candidate.setFromIK(group_, pose, "link_6", 0.05, validity)) continue;
      candidate = nearStart(candidate, start);
      if (!collision(scene, candidate).empty()) continue;
      const auto q = positions(candidate);
      bool duplicate = false;
      for (const auto& previous : accepted) {
        double d = 0;
        for (size_t i = 0; i < q.size(); ++i) d += std::pow(q[i] - previous[i], 2);
        if (d < 1e-8) duplicate = true;
      }
      if (duplicate) continue;
      accepted.push_back(q);
      double score = 0;
      for (size_t i = 0; i < q.size(); ++i) score += std::pow(q[i] - current[i], 2);
      if (score < best_score) { best_score = score; best = candidate; }
    }
    result.ik_candidates = accepted.size();
    if (accepted.empty()) throw PlanningError(Codes::NO_IK_SOLUTION, "No collision-free IK solution found within the seed budget");
    return best;
  }

  uint32_t validatePath(const std::shared_ptr<Handle>& handle, const std::shared_ptr<Scene>& scene,
                        const robot_trajectory::RobotTrajectory& trajectory)
  {
    uint32_t checked = 0;
    State sample(move_group_->getRobotModel());
    for (size_t i = 0; i < trajectory.getWayPointCount(); ++i) {
      const auto& end = trajectory.getWayPoint(i);
      const auto& begin = trajectory.getWayPoint(i ? i - 1 : 0);
      const auto a = positions(begin), b = positions(end);
      double max_delta = 0;
      for (size_t j = 0; j < a.size(); ++j) max_delta = std::max(max_delta, std::abs(b[j] - a[j]));
      const int subdivisions = std::max(1, static_cast<int>(std::ceil(max_delta / step_)));
      for (int k = 0; k <= subdivisions; ++k) {
        checkCanceled(handle);
        begin.interpolate(end, static_cast<double>(k) / subdivisions, sample);
        sample.update();
        const auto issue = collision(scene, sample);
        if (!issue.empty()) throw PlanningError(Codes::INVALID_MOTION_PLAN, "Trajectory validation: " + issue);
        ++checked;
        if (checked > 100000) throw PlanningError(Codes::INVALID_MOTION_PLAN, "Trajectory validation budget exceeded");
      }
    }
    return checked;
  }

  void run(const std::shared_ptr<Handle>& handle)
  {
    auto result = std::make_shared<Action::Result>();
    try {
      checkCanceled(handle);
      feedback(handle, "checking_start", "Reading joint feedback and planning scene");
      auto start = move_group_->getCurrentState(3.0);
      if (!start) throw PlanningError(Codes::ROBOT_STATE_STALE, "No current six-joint state available");
      start->update();
      auto scene = snapshot();
      const auto start_issue = collision(scene, *start);
      if (!start_issue.empty()) throw PlanningError(Codes::START_STATE_IN_COLLISION, "Start state: " + start_issue);
      State target(*start);
      const auto goal = handle->get_goal();
      if (goal->target_type == Action::Goal::JOINT) {
        target.setJointGroupPositions(group_, goal->joint_positions.data());
        target = nearStart(target, *start);
      } else {
        feedback(handle, "solving_ik", "Selecting a nearby collision-free IK candidate");
        target = poseTarget(handle, *start, scene, *result);
      }
      const auto target_issue = collision(scene, target);
      if (!target_issue.empty()) throw PlanningError(Codes::GOAL_IN_COLLISION, "Goal state: " + target_issue);
      const auto selected = positions(target);
      std::copy(selected.begin(), selected.end(), result->selected_joint_positions.begin());
      checkCanceled(handle);
      move_group_->setStartState(*start);
      move_group_->setJointValueTarget(target);
      move_group_->setPlanningTime(goal->planning_time > 0 ? goal->planning_time : 5.0);
      move_group_->setMaxVelocityScalingFactor(goal->velocity_scaling > 0 ? goal->velocity_scaling : 0.2);
      move_group_->setMaxAccelerationScalingFactor(goal->acceleration_scaling > 0 ? goal->acceleration_scaling : 0.2);
      feedback(handle, "planning", "OMPL RRTConnect");
      moveit::planning_interface::MoveGroupInterface::Plan plan;
      const auto status = move_group_->plan(plan);
      checkCanceled(handle);
      if (status.val != Codes::SUCCESS) throw PlanningError(status.val, "MoveIt could not find a path");
      result->planning_time = plan.planning_time_;
      robot_trajectory::RobotTrajectory trajectory(move_group_->getRobotModel(), "arm_control");
      trajectory.setRobotTrajectoryMsg(*start, plan.trajectory_);
      trajectory.unwind(*start);
      if (!trajectory.getWayPointCount()) throw PlanningError(Codes::INVALID_MOTION_PLAN, "Empty trajectory");
      feedback(handle, "validating", "Checking interpolated states against the latest scene");
      scene = snapshot();
      result->validation_samples = validatePath(handle, scene, trajectory);
      for (size_t i = 1; i < trajectory.getWayPointCount(); ++i)
        result->joint_path_length += trajectory.getWayPoint(i - 1).distance(trajectory.getWayPoint(i), group_);
      trajectory.getRobotTrajectoryMsg(plan.trajectory_);
      result->trajectory = plan.trajectory_;
      moveit_msgs::msg::DisplayTrajectory display;
      display.model_id = move_group_->getRobotModel()->getName();
      display.trajectory_start = plan.start_state_;
      display.trajectory.push_back(plan.trajectory_);
      display_->publish(display);
      if (goal->execute) {
        auto current = move_group_->getCurrentState(2.0);
        if (!current || current->distance(*start, group_) > 0.01)
          throw PlanningError(Codes::ROBOT_STATE_STALE, "Robot moved after planning; submit a new request");
        checkCanceled(handle);
        feedback(handle, "executing", "Executing with the demo mock controller");
        const auto execution = move_group_->execute(plan);
        checkCanceled(handle);
        if (execution.val != Codes::SUCCESS) throw PlanningError(execution.val, "Trajectory execution failed");
      }
      result->success = true;
      result->error_code = Codes::SUCCESS;
      result->message = goal->execute ? "Planned, validated and executed in the mock demo" : "Planned and validated; execution not requested";
    } catch (const PlanningError& error) {
      result->error_code = error.code;
      result->message = error.what();
    } catch (const std::exception& error) {
      result->error_code = Codes::FAILURE;
      result->message = error.what();
    }
    RCLCPP_INFO(node_->get_logger(), "Result %d: %s", result->error_code, result->message.c_str());
    if (rclcpp::ok()) {
      if (handle->is_canceling()) { result->success = false; result->error_code = Codes::PREEMPTED; handle->canceled(result); }
      else if (result->success) handle->succeed(result);
      else handle->abort(result);
    }
    cancel_requested_.store(false);
    busy_.store(false);
  }

  rclcpp::Node::SharedPtr node_;
  std::unique_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;
  const moveit::core::JointModelGroup* group_{};
  rclcpp::Client<moveit_msgs::srv::GetPlanningScene>::SharedPtr scene_client_;
  rclcpp::Publisher<moveit_msgs::msg::DisplayTrajectory>::SharedPtr display_;
  rclcpp_action::Server<Action>::SharedPtr server_;
  rclcpp::TimerBase::SharedPtr cancel_timer_;
  std::atomic<bool> busy_{false};
  std::atomic<bool> cancel_requested_{false};
  std::thread worker_;
  double step_;
  int ik_attempts_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("arm_planning", rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));
  int result = 0;
  try {
    ArmPlanner planner(node);
    rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 4);
    executor.add_node(node);
    executor.spin();
  } catch (const std::exception& error) {
    RCLCPP_ERROR(node->get_logger(), "%s", error.what());
    result = 1;
  }
  if (rclcpp::ok()) rclcpp::shutdown();
  return result;
}
