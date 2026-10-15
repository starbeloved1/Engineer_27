#include "arm_goal/goal_client_node.hpp"

#include <moveit_msgs/msg/move_it_error_codes.hpp>
#include <chrono>
#include <thread>

namespace arm_goal
{
using Codes = moveit_msgs::msg::MoveItErrorCodes;

void GoalClientNode::configureTopics()
{
  auto topic = [this](const std::string& key)
  {
    const auto value = get_parameter(key).as_string();
    if (value.empty())
      throw std::invalid_argument("Empty Topic parameter: " + key);
    return value;
  };
  // 命令不持久化，避免重新启动后执行历史目标。回调和 Action 由同一执行器串行处理。
  const auto qos = rclcpp::QoS(10).reliable().durability_volatile();
  feedback_publisher_ = create_publisher<eng_interfaces::msg::MotionFeedback>(topic("feedback_topic"), qos);
  result_publisher_ = create_publisher<eng_interfaces::msg::MotionResult>(topic("result_topic"), qos);
  target_subscription_ = create_subscription<Target>(
      topic("target_topic"), qos, [this](Target::ConstSharedPtr message) { receiveTarget(*message); });
  cancel_subscription_ = create_subscription<std_msgs::msg::String>(
      topic("cancel_topic"), qos,
      [this](std_msgs::msg::String::ConstSharedPtr message)
      {
        if (busy_ && message->data == active_id_)
        {
          cancel_requested_ = true;
          publishFeedback(active_id_, "canceling", "Cancellation requested by Topic");
        }
        else
          publishFeedback(message->data, "cancel_ignored", "No matching active request");
      });
  RCLCPP_INFO(get_logger(), "Topic input ready: %s -> %s", topic("target_topic").c_str(),
              get_parameter("planning_action").as_string().c_str());
}

void GoalClientNode::receiveTarget(const Target& message)
{
  if (message.request_id.empty())
  {
    publishError(message.request_id, Codes::INVALID_GOAL_CONSTRAINTS, "request_id must not be empty");
    return;
  }
  if (busy_)
  {
    if (message.request_id == active_id_)
      publishFeedback(message.request_id, "duplicate_ignored", "This request is already active");
    else
      publishError(message.request_id, Codes::FAILURE, "Topic client busy; target rejected, not queued");
    return;
  }
  active_id_ = message.request_id;
  cancel_requested_ = false;
  result_published_ = false;
  busy_ = true;
  pending_ = message;
  publishFeedback(active_id_, "accepted", "Topic target received; waiting for Action server acceptance");
}

void GoalClientNode::publishFeedback(const std::string& id, const std::string& stage,
                                      const std::string& detail)
{
  eng_interfaces::msg::MotionFeedback out;
  out.request_id = id;
  out.stage = stage;
  out.detail = detail;
  feedback_publisher_->publish(out);
}

void GoalClientNode::publishResult(const std::string& id, const Action::Result& result)
{
  eng_interfaces::msg::MotionResult out;
  out.request_id = id;
  out.success = result.success;
  out.error_code = result.error_code;
  out.message = result.message;
  out.trajectory = result.trajectory;
  out.selected_joint_positions = result.selected_joint_positions;
  out.planning_time = result.planning_time;
  out.joint_path_length = result.joint_path_length;
  out.validation_samples = result.validation_samples;
  out.ik_candidates = result.ik_candidates;
  result_publisher_->publish(out);
  // 拒绝另一个请求时，不能把当前正在运行的请求标记为已完成。
  if (id == active_id_)
    result_published_ = true;
}

void GoalClientNode::publishError(const std::string& id, int32_t code, const std::string& message)
{
  Action::Result result;
  result.error_code = code;
  result.message = message;
  publishResult(id, result);
}

int GoalClientNode::listen(const std::function<bool()>& interrupted)
{
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(shared_from_this());
  while (rclcpp::ok() && !interrupted())
  {
    executor.spin_some();
    if (!pending_)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    const auto target = *pending_;
    pending_.reset();
    GoalOptions options;
    options.goal.target_type = target.target_type;
    options.goal.joint_positions = target.joint_positions;
    options.goal.target_pose = target.target_pose;
    options.goal.execute = target.execute;
    options.goal.velocity_scaling = target.velocity_scaling;
    options.goal.acceleration_scaling = target.acceleration_scaling;
    options.goal.planning_time = target.planning_time;
    try
    {
      const int code = submit(options, executor, [this, &interrupted]
                              { return interrupted() || cancel_requested_; });
      if (!result_published_)
        publishError(active_id_, code == 130 ? Codes::PREEMPTED : Codes::FAILURE,
                     code == 130 ? "Request canceled" : "Request did not complete");
    }
    catch (const std::exception& error)
    {
      const std::string message = error.what();
      const int32_t code = message.find("timed out") != std::string::npos ? Codes::TIMED_OUT
                           : message.find("unavailable") != std::string::npos ? Codes::COMMUNICATION_FAILURE
                           : Codes::FAILURE;
      publishError(active_id_, code, message);
    }
    busy_ = false;
    active_id_.clear();
    cancel_requested_ = false;
  }
  return 0;
}
}  // namespace arm_goal
