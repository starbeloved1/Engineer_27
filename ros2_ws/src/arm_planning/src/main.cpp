#include "arm_planning/arm_planning_node.hpp"
#include "arm_planning/config_arguments.hpp"

int main(int argc, char** argv)
{
  int result = 0;
  try
  {
    const auto arguments = arm_planning::argumentsWithConfig(argc, argv, "arm_planning.yaml");
    const auto pointers = arm_planning::argumentPointers(arguments);
    rclcpp::init(pointers.size(), pointers.data());
    auto node = rclcpp::Node::make_shared(
        "arm_planning",
        rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));
    arm_planning::ArmPlanningNode application(node);
    rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 4);
    executor.add_node(node);
    executor.spin();
  }
  catch (const std::exception& error)
  {
    RCLCPP_ERROR(rclcpp::get_logger("arm_planning"), "%s", error.what());
    result = 1;
  }
  if (rclcpp::ok())
    rclcpp::shutdown();
  return result;
}
