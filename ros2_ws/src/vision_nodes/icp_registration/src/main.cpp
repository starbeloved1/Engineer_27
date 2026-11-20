#include "icp_registration/icp_registration_node.hpp"
#include <thread>

int main(int argc, char** argv)
{
  try
  {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<icp_registration::IcpRegistrationNode>();
    rclcpp::executors::SingleThreadedExecutor executor;
    rclcpp::executors::SingleThreadedExecutor detection_executor;
    executor.add_node(node);
    detection_executor.add_callback_group(node->detectionGroup(), node->get_node_base_interface());
    std::thread detection_thread([&detection_executor]() { detection_executor.spin(); });
    try
    {
      executor.spin();
    }
    catch (...)
    {
      detection_executor.cancel();
      detection_thread.join();
      throw;
    }
    detection_executor.cancel();
    detection_thread.join();
    if (rclcpp::ok()) rclcpp::shutdown();
    return 0;
  }
  catch (const std::exception& error)
  {
    RCLCPP_ERROR(rclcpp::get_logger("icp_registration"), "%s", error.what());
    if (rclcpp::ok()) rclcpp::shutdown();
    return 1;
  }
}
