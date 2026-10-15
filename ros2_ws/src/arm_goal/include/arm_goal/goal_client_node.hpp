#pragma once

#include <act_interfaces/action/plan_motion.hpp>
#include <act_interfaces/msg/motion_target.hpp>
#include <act_interfaces/msg/motion_feedback.hpp>
#include <act_interfaces/msg/motion_result.hpp>
#include <std_msgs/msg/string.hpp>
#include <functional>
#include <optional>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <string>
#include <vector>

namespace arm_goal
{
struct GoalOptions
{
  act_interfaces::action::PlanMotion::Goal goal;
  std::string output;
  double timeout{0};
  bool help{false};
  bool listen{false};
};

GoalOptions parseGoalOptions(const std::vector<std::string>& arguments);
std::string goalUsage();

// 两种输入共用同一个 Action 提交流程。Topic 回调只接收/拒绝请求，不嵌套运行任务。
class GoalClientNode : public rclcpp::Node
{
 public:
  explicit GoalClientNode(bool listen = false);
  int run(const GoalOptions& options, const std::function<bool()>& interrupted);
  int listen(const std::function<bool()>& interrupted);

 private:
  using Action = act_interfaces::action::PlanMotion;
  using Handle = rclcpp_action::ClientGoalHandle<Action>;
  using Target = act_interfaces::msg::MotionTarget;
  void cancel(rclcpp::executors::SingleThreadedExecutor& executor, const Handle::SharedPtr& handle);
  int submit(const GoalOptions& options, rclcpp::executors::SingleThreadedExecutor& executor,
             const std::function<bool()>& interrupted);
  void configureTopics();
  void receiveTarget(const Target& message);
  void publishFeedback(const std::string& id, const std::string& stage, const std::string& detail);
  void publishResult(const std::string& id, const Action::Result& result);
  void publishError(const std::string& id, int32_t code, const std::string& message);

  rclcpp_action::Client<Action>::SharedPtr client_;
  double server_timeout_, acceptance_timeout_, request_timeout_, discovery_settle_time_;
  bool discovery_settled_{false};
  bool topic_mode_{false};
  // 同一 SingleThreadedExecutor 中处理订阅和 Action 回调；等待时持续 spin_some。
  bool busy_{false}, cancel_requested_{false}, result_published_{false};
  std::string active_id_;
  std::optional<Target> pending_;
  rclcpp::Subscription<Target>::SharedPtr target_subscription_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr cancel_subscription_;
  rclcpp::Publisher<act_interfaces::msg::MotionFeedback>::SharedPtr feedback_publisher_;
  rclcpp::Publisher<act_interfaces::msg::MotionResult>::SharedPtr result_publisher_;
};
}  // namespace arm_goal
