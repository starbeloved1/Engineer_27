#!/usr/bin/env bash
# 在 Ubuntu 22.04 电脑本地终端运行；Jetson 负责运行 ./run.sh use_rviz:=false。
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ "${1:-}" == --help || "${1:-}" == -h ]]; then
  echo '用法：./run_rviz.sh [ROS launch 参数]'
  echo '仅启动 MTC RViz，显示取矿／存车阶段、当前 TCP 目标和槽内矿。'
  echo 'ROS_DOMAIN_ID=6，允许跨机器 ROS 通信。'
  echo '先在电脑构建：./build.sh --packages-up-to sim_controller --cmake-args -DBUILD_TESTING=OFF'
  exit 0
fi
if [[ ! -f "$engineer_root/ros2_ws/install/setup.bash" ]]; then
  echo '请先执行 ./build.sh --packages-up-to sim_controller --cmake-args -DBUILD_TESTING=OFF' >&2
  exit 1
fi
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
source "$engineer_root/ros2_ws/install/setup.bash"
source "$engineer_root/tools/internal/ros_env.sh"
export ROS_LOCALHOST_ONLY=0
export RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION:-rmw_fastrtps_cpp}"
if [[ -z "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]]; then
  echo '请在电脑本地桌面的终端运行，需要可用的图形显示环境。' >&2
  exit 1
fi
exec /usr/bin/python3 /opt/ros/humble/bin/ros2 launch sim_controller remote_rviz.launch.py "$@"
