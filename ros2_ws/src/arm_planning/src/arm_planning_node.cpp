#include "arm_planning/arm_planning_node.hpp"

#include <chrono>

namespace arm_planning
{
using namespace std::chrono_literals;
ArmPlanningNode::ArmPlanningNode(const rclcpp::Node::SharedPtr& node)
    : node_(node), settings_(node), planner_(node, settings_)
{
  server_ = rclcpp_action::create_server<Action>(
      node_, settings_.planning_action,
      [this](const rclcpp_action::GoalUUID&, const std::shared_ptr<const Action::Goal> goal)
      {
        if (!planner_.validGoal(*goal))
          return rclcpp_action::GoalResponse::REJECT;
        if (busy_.load())
          return rclcpp_action::GoalResponse::REJECT;
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
      },
      [this](const std::shared_ptr<Handle>)
      {
        cancel_requested_.store(true);
        return rclcpp_action::CancelResponse::ACCEPT;
      },
      [this](const std::shared_ptr<Handle> handle)
      {
        // 同一互斥回调组串行处理目标接收；成功发送接收响应后才占用任务槽。
        // 若在 goal 回调中提前置 busy，响应发送失败时不会进入这里，节点会永久忙碌。
        busy_.store(true);
        if (worker_.joinable())
          worker_.join();
        worker_ = std::thread([this, handle] { run(handle); });
      });
  // 与 Action 共用互斥回调组，进入 CANCELING 后持续停止直到任务退出
  // 单次 stop 可能早于控制器启动而失效，不能只发送一次
  cancel_timer_ = node_->create_wall_timer(20ms,
                                           [this]
                                           {
                                             if (cancel_requested_.load())
                                               planner_.stop();
                                           });
  RCLCPP_INFO(node_->get_logger(), "Ready: %s; group=%s; tip=%s", settings_.planning_action.c_str(),
              settings_.planning_group.c_str(), settings_.end_effector_link.c_str());
}

ArmPlanningNode::~ArmPlanningNode()
{
  if (worker_.joinable())
  {
    if (rclcpp::ok())
      planner_.stop();
    worker_.join();
  }
}

void ArmPlanningNode::publishFeedback(const std::shared_ptr<Handle>& handle,
                                      const std::string& stage, const std::string& detail)
{
  auto message = std::make_shared<Action::Feedback>();
  message->stage = stage;
  message->detail = detail;
  handle->publish_feedback(message);
}

void ArmPlanningNode::checkCanceled(const std::shared_ptr<Handle>& handle)
{
  if (handle->is_canceling() || !rclcpp::ok())
    throw PlanningError(Codes::PREEMPTED, "Request canceled");
}

void ArmPlanningNode::run(const std::shared_ptr<Handle>& handle)
{
  auto result = std::make_shared<Action::Result>();
  try
  {
    planner_.plan(
        *handle->get_goal(), *result,
        [this, &handle](const auto& stage, const auto& detail)
        { publishFeedback(handle, stage, detail); },
        [&handle] { checkCanceled(handle); });
  }
  catch (const PlanningError& error)
  {
    result->error_code = error.code;
    result->message = error.what();
  }
  catch (const std::exception& error)
  {
    result->error_code = Codes::FAILURE;
    result->message = error.what();
  }
  RCLCPP_INFO(node_->get_logger(), "Result %d: %s", result->error_code, result->message.c_str());
  if (rclcpp::ok())
  {
    if (handle->is_canceling())
    {
      result->success = false;
      result->error_code = Codes::PREEMPTED;
      handle->canceled(result);
    }
    else if (result->success)
      handle->succeed(result);
    else
      handle->abort(result);
  }
  cancel_requested_.store(false);
  busy_.store(false);
}
}  // namespace arm_planning
