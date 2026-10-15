#include <cmath>
#include <stdexcept>

#include "arm_goal/goal_client_node.hpp"

namespace arm_goal
{
std::string goalUsage()
{
  return "plan_goal --listen  # 常驻订阅 Topic，参数来自 arm_goal.yaml\n"
         "plan_goal (--joints J1 J2 J3 J4 J5 J6 | --pose X Y Z QX QY QZ QW)\n"
         "  [--frame base_link|world] [--execute] [--planning-time SEC]\n"
         "  [--velocity-scale SCALE] [--acceleration-scale SCALE]\n"
         "  [--timeout SEC] [--output FILE] [--ros-args ...]\n"
         "角度为 rad，位置为 m；默认只规划，0 缩放或规划时间采用服务端 YAML "
         "默认值";
}

GoalOptions parseGoalOptions(const std::vector<std::string>& args)
{
  GoalOptions out;
  bool has_target = false;
  out.goal.target_pose.header.frame_id = "base_link";
  for (size_t i = 1; i < args.size(); ++i)
  {
    const auto option = args[i];
    auto next = [&]() -> std::string
    {
      if (++i >= args.size())
        throw std::invalid_argument("Missing value after " + option);
      return args[i];
    };
    auto number = [&]()
    {
      const auto text = next();
      size_t used = 0;
      const double value = std::stod(text, &used);
      if (used != text.size() || !std::isfinite(value))
        throw std::invalid_argument("Invalid number: " + text);
      return value;
    };
    if (option == "--help" || option == "-h")
    {
      out.help = true;
      return out;
    }
    if (option == "--listen")
    {
      out.listen = true;
    }
    else if (option == "--joints" || option == "--pose")
    {
      if (has_target)
        throw std::invalid_argument("Choose exactly one of --joints or --pose");
      has_target = true;
      if (option == "--joints")
      {
        out.goal.target_type = out.goal.JOINT;
        for (auto& q : out.goal.joint_positions) q = number();
      }
      else
      {
        out.goal.target_type = out.goal.POSE;
        auto& p = out.goal.target_pose.pose.position;
        auto& q = out.goal.target_pose.pose.orientation;
        p.x = number();
        p.y = number();
        p.z = number();
        q.x = number();
        q.y = number();
        q.z = number();
        q.w = number();
      }
    }
    else if (option == "--frame")
      out.goal.target_pose.header.frame_id = next();
    else if (option == "--execute")
      out.goal.execute = true;
    else if (option == "--planning-time")
      out.goal.planning_time = number();
    else if (option == "--velocity-scale")
      out.goal.velocity_scaling = number();
    else if (option == "--acceleration-scale")
      out.goal.acceleration_scaling = number();
    else if (option == "--timeout")
    {
      out.timeout = number();
      if (out.timeout <= 0)
        throw std::invalid_argument("Timeout must be positive");
    }
    else if (option == "--output")
      out.output = next();
    else
      throw std::invalid_argument("Unknown option: " + option);
  }
  if (out.listen)
  {
    if (args.size() != 2)
      throw std::invalid_argument("--listen only accepts ROS parameters; submit targets via Topic");
    return out;
  }
  if (!has_target)
    throw std::invalid_argument("A --joints or --pose target is required");
  if (out.goal.planning_time < 0 || out.goal.planning_time > 30 || out.goal.velocity_scaling < 0 ||
      out.goal.velocity_scaling > 1 || out.goal.acceleration_scaling < 0 ||
      out.goal.acceleration_scaling > 1)
    throw std::invalid_argument("Planning time must be in [0,30], scaling in [0,1]");
  const auto& frame = out.goal.target_pose.header.frame_id;
  if (frame != "base_link" && frame != "world")
    throw std::invalid_argument("Unsupported target frame");
  if (out.goal.target_type == out.goal.POSE)
  {
    const auto& q = out.goal.target_pose.pose.orientation;
    if (std::abs(std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w) - 1) >= .01)
      throw std::invalid_argument("Pose orientation must be a unit quaternion");
  }
  return out;
}
}  // namespace arm_goal
