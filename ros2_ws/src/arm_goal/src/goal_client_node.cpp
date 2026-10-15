#include "arm_goal/goal_client_node.hpp"

#include <action_msgs/msg/goal_status.hpp>
#include <json/json.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <thread>

namespace arm_goal
{
namespace
{
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

template <class Future>
bool wait(rclcpp::executors::SingleThreadedExecutor& executor, Future& future, double seconds,
          const std::function<bool()>& interrupted)
{
  const auto deadline = Clock::now() + std::chrono::duration<double>(seconds);
  while (rclcpp::ok() && Clock::now() < deadline && !interrupted())
  {
    executor.spin_some();
    if (future.wait_for(0s) == std::future_status::ready)
      return true;
    std::this_thread::sleep_for(10ms);
  }
  return future.wait_for(0s) == std::future_status::ready;
}

template <class Values>
Json::Value array(const Values& values)
{
  Json::Value out(Json::arrayValue);
  for (const auto& value : values) out.append(value);
  return out;
}

Json::Value resultJson(const eng_interfaces::action::PlanMotion::Result& result)
{
  Json::Value out;
  out["success"] = result.success;
  out["error_code"] = result.error_code;
  out["message"] = result.message;
  out["selected_joint_positions"] = array(result.selected_joint_positions);
  out["planning_time"] = result.planning_time;
  out["joint_path_length"] = result.joint_path_length;
  out["validation_samples"] = result.validation_samples;
  out["ik_candidates"] = result.ik_candidates;
  const auto& trajectory = result.trajectory.joint_trajectory;
  out["joint_names"] = array(trajectory.joint_names);
  out["points"] = Json::Value(Json::arrayValue);
  for (const auto& point : trajectory.points)
  {
    Json::Value p;
    p["positions"] = array(point.positions);
    p["velocities"] = array(point.velocities);
    p["accelerations"] = array(point.accelerations);
    p["time_from_start"] = point.time_from_start.sec + point.time_from_start.nanosec * 1e-9;
    out["points"].append(p);
  }
  return out;
}
}  // namespace

GoalClientNode::GoalClientNode(bool listen)
    : Node(listen ? "arm_goal" : "arm_plan_goal",
           rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true))
{
  const auto action = get_parameter("planning_action").as_string();
  server_timeout_ = get_parameter("server_timeout").as_double();
  acceptance_timeout_ = get_parameter("acceptance_timeout").as_double();
  request_timeout_ = get_parameter("request_timeout").as_double();
  discovery_settle_time_ = has_parameter("discovery_settle_time")
                               ? get_parameter("discovery_settle_time").as_double()
                               : declare_parameter<double>("discovery_settle_time", 0.5);
  if (action.empty() || !std::isfinite(server_timeout_) || server_timeout_ <= 0 ||
      !std::isfinite(acceptance_timeout_) || acceptance_timeout_ <= 0 ||
      !std::isfinite(request_timeout_) || request_timeout_ <= 0 ||
      !std::isfinite(discovery_settle_time_) || discovery_settle_time_ < 0 || discovery_settle_time_ > 5)
    throw std::invalid_argument("Invalid plan_goal YAML parameters");
  client_ = rclcpp_action::create_client<Action>(this, action);
  topic_mode_ = listen;
  if (topic_mode_)
    configureTopics();
}

void GoalClientNode::cancel(rclcpp::executors::SingleThreadedExecutor& executor,
                            const Handle::SharedPtr& handle)
{
  if (!handle || !rclcpp::ok())
    return;
  try
  {
    const auto status = handle->get_status();
    if (status == action_msgs::msg::GoalStatus::STATUS_SUCCEEDED ||
        status == action_msgs::msg::GoalStatus::STATUS_ABORTED ||
        status == action_msgs::msg::GoalStatus::STATUS_CANCELED)
      return;
    // 先取得结果 future：等待取消响应时可能已处理结果，届时句柄会从 client 中移除。
    auto finished = client_->async_get_result(handle);
    auto future = client_->async_cancel_goal(handle);
    wait(executor, future, 3., [] { return false; });
    // 等待服务端完成取消，下一条 Topic 目标不与上一次停止过程重叠。
    wait(executor, finished, 3., [] { return false; });
  }
  catch (const std::exception& error)
  {
    std::cerr << "Cancellation request failed: " << error.what() << '\n';
  }
}

int GoalClientNode::run(const GoalOptions& options, const std::function<bool()>& interrupted)
{
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(shared_from_this());
  return submit(options, executor, interrupted);
}

int GoalClientNode::submit(const GoalOptions& options,
                           rclcpp::executors::SingleThreadedExecutor& executor,
                           const std::function<bool()>& interrupted)
{
  if (!client_->action_server_is_ready())
    discovery_settled_ = false;
  const auto deadline = Clock::now() + std::chrono::duration<double>(server_timeout_);
  while (rclcpp::ok() && !client_->action_server_is_ready() && Clock::now() < deadline &&
         !interrupted())
  {
    executor.spin_some();
    std::this_thread::sleep_for(20ms);
  }
  if (interrupted())
    return 130;
  if (!client_->action_server_is_ready())
    throw std::runtime_error(
        "Planning server unavailable; start "
        "eng_bringup offline.launch.py");
  if (!discovery_settled_)
  {
    // 本机 Fast DDS 可先发现服务，再匹配响应 reader；不重发运动目标，先等待匹配稳定。
    const auto ready = Clock::now() + std::chrono::duration<double>(discovery_settle_time_);
    while (rclcpp::ok() && Clock::now() < ready && !interrupted())
    {
      executor.spin_some();
      std::this_thread::sleep_for(10ms);
    }
    if (interrupted())
      return 130;
    discovery_settled_ = true;
  }
  rclcpp_action::Client<Action>::SendGoalOptions callbacks;
  const auto id = active_id_;
  callbacks.feedback_callback =
      [this, id](Handle::SharedPtr, const std::shared_ptr<const Action::Feedback> feedback)
  {
    if (topic_mode_)
      publishFeedback(id, feedback->stage, feedback->detail);
    else
      std::cout << feedback->stage << ": " << feedback->detail << std::endl;
  };
  // 接收超时后若仍收到迟到的句柄，应取消该任务，不能留下无人管理的目标。
  auto abandoned = std::make_shared<bool>(false);
  callbacks.goal_response_callback = [this, abandoned](Handle::SharedPtr handle)
  {
    if (*abandoned && handle)
      client_->async_cancel_goal(handle);
  };
  auto sent = client_->async_send_goal(options.goal, callbacks);
  // 等待接收期间也保留 ROS 上下文，取得句柄后才能正确取消已发出的任务
  if (!wait(executor, sent, acceptance_timeout_, [] { return false; }))
  {
    *abandoned = true;
    throw std::runtime_error("Goal acceptance timed out");
  }
  const auto handle = sent.get();
  if (!handle)
    throw std::runtime_error("Goal rejected: server busy or invalid request");
  if (interrupted())
  {
    cancel(executor, handle);
    return 130;
  }
  auto future = client_->async_get_result(handle);
  if (!wait(executor, future, options.timeout > 0 ? options.timeout : request_timeout_,
            interrupted))
  {
    cancel(executor, handle);
    if (interrupted())
      return 130;
    throw std::runtime_error("Request timed out; cancellation requested");
  }
  if (interrupted())
  {
    cancel(executor, handle);
    return 130;
  }
  const auto result = future.get().result;
  if (topic_mode_)
  {
    publishResult(id, *result);
    return result->success ? 0 : 1;
  }
  const auto data = resultJson(*result);
  auto summary = data;
  summary.removeMember("points");
  std::cout << summary.toStyledString() << "Trajectory points: " << data["points"].size()
            << std::endl;
  if (!options.output.empty())
  {
    const auto path = std::filesystem::path(options.output);
    if (!path.parent_path().empty())
      std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path);
    if (!file || !(file << data.toStyledString()))
      throw std::runtime_error("Cannot write output: " + options.output);
  }
  return result->success ? 0 : 1;
}
}  // namespace arm_goal
