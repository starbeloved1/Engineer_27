#!/usr/bin/env bash
# 独立查看机械臂、D435 外壳与 TF；不启动相机、控制器、视觉或规划。
set -eo pipefail
tools_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ "${1:-}" == --help || "${1:-}" == -h ]]; then
  echo '用法：./tools/preview_camera.sh [gui:=false] [rviz:=false]'
  echo '默认打开 RViz 和关节滑块。相机长×深×高：90×25×25 mm。'
  echo '沿用当前 URDF 安装关系；外壳与光学坐标系为示意，不代表实测标定。'
  echo '独立本机 ROS 域默认 86，可用 D435_PREVIEW_DOMAIN_ID 覆盖。'
  exit 0
fi
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
export ROS_DOMAIN_ID="${D435_PREVIEW_DOMAIN_ID:-86}"
export ROS_LOCALHOST_ONLY=1
exec /usr/bin/python3 /opt/ros/humble/bin/ros2 launch "$tools_dir/preview.launch.py" \
  camera:=true "rviz_config:=$tools_dir/camera_preview.rviz" "$@"
