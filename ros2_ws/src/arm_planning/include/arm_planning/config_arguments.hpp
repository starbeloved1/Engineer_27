#pragma once

#include <string>
#include <vector>

namespace arm_planning
{
// 默认 YAML 先于用户的 ROS 参数加载，后传入的参数可覆盖默认配置
std::vector<std::string> argumentsWithConfig(int argc, char** argv, const std::string& config_name);
std::vector<const char*> argumentPointers(const std::vector<std::string>& arguments);
}  // namespace arm_planning
