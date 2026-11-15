#pragma once
#include <rclcpp/rclcpp.hpp>

namespace vision_core
{
template <class Node>
int runNode(int argc, char** argv, const char* name)
{
  int result = 0;
  try
  {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Node>());
  }
  catch (const std::exception& error)
  {
    RCLCPP_ERROR(rclcpp::get_logger(name), "%s", error.what());
    result = 1;
  }
  if (rclcpp::ok())
    rclcpp::shutdown();
  return result;
}
}  // namespace vision_core
