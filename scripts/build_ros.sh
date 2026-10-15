#!/usr/bin/env bash
# 运行产物放入 .cache，不复用仓库中已有的历史 build/install
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
# 只在本脚本进程中隔离旧工作区和 Python 环境的 ROS 路径
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
/usr/bin/colcon --log-base "$engineer_root/.cache/ros2/log" build \
  --base-paths "$engineer_root/ros2_ws/src" \
  --build-base "$engineer_root/.cache/ros2/build" \
  --install-base "$engineer_root/.cache/ros2/install" \
  --symlink-install \
  --cmake-args -DPython3_EXECUTABLE=/usr/bin/python3 "$@"
