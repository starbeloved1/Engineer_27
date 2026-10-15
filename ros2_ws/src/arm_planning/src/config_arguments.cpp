#include "arm_planning/config_arguments.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>

namespace arm_planning
{
std::vector<std::string> argumentsWithConfig(int argc, char** argv, const std::string& config_name)
{
  std::vector<std::string> result{
      argv[0], "--ros-args", "--params-file",
      ament_index_cpp::get_package_share_directory("arm_planning") + "/config/" + config_name,
      "--"};
  for (int i = 1; i < argc; ++i) result.emplace_back(argv[i]);
  return result;
}
std::vector<const char*> argumentPointers(const std::vector<std::string>& arguments)
{
  std::vector<const char*> result;
  for (const auto& value : arguments) result.push_back(value.c_str());
  return result;
}
}  // namespace arm_planning
