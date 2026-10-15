#include <algorithm>
#include <csignal>
#include <iostream>

#include "arm_goal/config_arguments.hpp"
#include "arm_goal/goal_client_node.hpp"

namespace
{
volatile std::sig_atomic_t interrupted = 0;
void receiveSignal(int)
{
  interrupted = 1;
}
}  // namespace

int main(int argc, char** argv)
{
  int result = 2;
  try
  {
    const auto clean = rclcpp::remove_ros_arguments(argc, argv);
    const bool listen = std::find(clean.begin(), clean.end(), "--listen") != clean.end();
    const auto arguments = arm_goal::argumentsWithConfig(
        argc, argv, listen ? "arm_goal.yaml" : "plan_goal.yaml");
    const auto pointers = arm_goal::argumentPointers(arguments);
    // Ctrl+C 先取消 Action，再关闭 ROS，避免任务留在服务端继续执行
    rclcpp::init(pointers.size(), pointers.data(), rclcpp::InitOptions(),
                 rclcpp::SignalHandlerOptions::None);
    std::signal(SIGINT, receiveSignal);
    std::signal(SIGTERM, receiveSignal);
    const auto options = arm_goal::parseGoalOptions(
        rclcpp::remove_ros_arguments(pointers.size(), pointers.data()));
    if (options.help)
    {
      std::cout << arm_goal::goalUsage() << std::endl;
      result = 0;
    }
    else
    {
      auto node = std::make_shared<arm_goal::GoalClientNode>(options.listen);
      result = options.listen ? node->listen([] { return interrupted != 0; })
                              : node->run(options, [] { return interrupted != 0; });
    }
  }
  catch (const std::exception& error)
  {
    std::cerr << error.what() << '\n';
  }
  if (rclcpp::ok())
    rclcpp::shutdown();
  return result;
}
