#!/usr/bin/env bash
# 查看完整 URDF、坐标系和关节滑块
set -eo pipefail
tools_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ "${1:-}" == --help || "${1:-}" == -h ]]; then
  echo '用法：./tools/preview_robot.sh [urdf:=FILE] [gui:=false] [rviz:=false]'
  echo '默认加载 assets/engineer/arm.urdf，打开 RViz 和关节角度滑块窗口'
  echo '坐标轴为 X 红、Y 绿、Z 蓝，显示所有 URDF 坐标系名称'
  echo '滑块调整模型关节角度，旋转关节数值单位为弧度；Center 回到滑块中位'
  echo '独立本机 ROS 域默认 85，可用 ROBOT_PREVIEW_DOMAIN_ID 覆盖'
  exit 0
fi
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
export ROS_DOMAIN_ID="${ROBOT_PREVIEW_DOMAIN_ID:-85}"
export ROS_LOCALHOST_ONLY=1
exec /usr/bin/python3 /opt/ros/humble/bin/ros2 launch "$tools_dir/preview.launch.py" "$@"
