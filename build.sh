#!/usr/bin/env bash
# 与 run.sh 同级；构建结果使用标准 ros2_ws/build、install、log 目录。
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# 只隔离本构建进程继承的旧工作区路径，确保安装环境不串到历史 .cache。
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
cd "$engineer_root/ros2_ws"
# Humble 消息生成器还使用旧版 FindPythonInterp，需要同时指定 PYTHON_EXECUTABLE。
/usr/bin/colcon build --symlink-install --cmake-args \
  -DPython3_EXECUTABLE=/usr/bin/python3 \
  -DPYTHON_EXECUTABLE=/usr/bin/python3 "$@"
