#include "engineer_mtc/mtc_node.hpp"

int main(int argc, char** argv)
{
  int status = 0;
  try
  {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<engineer_mtc::MtcNode>(
        rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));
    rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 3);
    executor.add_node(node);
    executor.spin();
  }
  catch (const std::exception& error)
  {
    RCLCPP_ERROR(rclcpp::get_logger("engineer_mtc"), "%s", error.what());
    status = 1;
  }
  if (rclcpp::ok()) rclcpp::shutdown();
  return status;
}
