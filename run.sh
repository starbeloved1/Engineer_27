#!/usr/bin/env bash
# 统一入口；各包自己的 launch 读取所属 YAML，bringup 只负责组合。
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ ! -f "$engineer_root/.cache/ros2/install/setup.bash" ]]; then
  echo "请先运行 ./scripts/build_ros.sh" >&2
  exit 1
fi
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
source "$engineer_root/.cache/ros2/install/setup.bash"
exec /usr/bin/python3 /opt/ros/humble/bin/ros2 launch eng_bringup offline.launch.py "$@"
