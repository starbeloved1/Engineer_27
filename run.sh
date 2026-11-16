#!/usr/bin/env bash
# 总启动只在这里组织：各功能包的 launch 自行读取所属 YAML。
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'HELP'
用法：./run.sh [mode:=mtc|vision] [use_rviz:=true|false]
               [controller_config:=FILE] [sim_config:=FILE] [mtc_config:=FILE]
               [start_camera:=true|false] [start_detector:=true|false] [start_icp:=true|false] [start_bridge:=true|false]
               [camera_config:=FILE] [vision_config:=FILE] [foxglove_config:=FILE]
               [yolo_python:=FILE]
默认 mtc：启动模拟控制、MoveIt、engineer_mtc 和任务 RViz；只规划预览，不执行运动。
机械臂 RViz 默认由 sim_controller/config/sim_controller.yaml 控制，use_rviz 可临时覆盖。
vision：启动 vision_nodes 分组下的 D435、YOLO/叠框图像、PCL ICP、Bridge 和录像；请先关闭 RealSense Viewer。
vision_pcl：保留为 vision 的兼容别名；各节点加载各自包内的 config。
Foxglove 连接 ws://127.0.0.1:8765；vision 加 use_rviz:=true 打开相机视角 RViz，默认不打开。
已有视觉流程时可单独执行 ros2 launch camera_driver vision_rviz.launch.py，仅打开显示窗口。
各包 launch 独立加载默认 YAML，配置路径参数仅用于显式覆盖；Ctrl+C 统一退出。
HELP
}

mode=mtc
vision_use_rviz=false
controller_arguments=()
mtc_arguments=()
start_camera=true
start_detector=true
start_icp=true
start_bridge=true
camera_arguments=()
detector_arguments=()
model_arguments=()
icp_arguments=()
bridge_arguments=()
recorder_arguments=()
for argument in "$@"; do
  case "$argument" in
    -h|--help) usage; exit 0 ;;
    mode:=vision|mode:=mtc) mode="${argument#*:=}" ;;
    mode:=vision_pcl) mode=vision ;;
    use_rviz:=true|use_rviz:=false)
      controller_arguments+=("$argument")
      vision_use_rviz="${argument#*:=}" ;;
    start_camera:=true|start_camera:=false) start_camera="${argument#*:=}" ;;
    start_detector:=true|start_detector:=false) start_detector="${argument#*:=}" ;;
    start_icp:=true|start_icp:=false) start_icp="${argument#*:=}" ;;
    start_bridge:=true|start_bridge:=false) start_bridge="${argument#*:=}" ;;
    controller_config:=?*) controller_arguments+=("$argument") ;;
    sim_config:=?*) controller_arguments+=("$argument") ;;
    mtc_config:=?*) mtc_arguments=("$argument") ;;
    camera_config:=?*) camera_arguments+=("$argument"); recorder_arguments+=("$argument") ;;
    vision_config:=?*)
      detector_arguments+=("$argument")
      model_arguments+=("$argument")
      icp_arguments+=("$argument") ;;
    foxglove_config:=?*) bridge_arguments+=("$argument") ;;
    yolo_python:=?*) detector_arguments+=("$argument") ;;
    *) echo "未知或无效参数：$argument" >&2; usage >&2; exit 2 ;;
  esac
done

# 同一工作区、同一 ROS 域只允许一套视觉流程，提前阻止相机和端口争用
if [[ "$mode" == vision ]]; then
  # 统一视觉启动目录；各包自行解析其配置中的资产路径
  cd "$engineer_root"
  mkdir -p "$engineer_root/.cache"
  exec {vision_lock_fd}>"$engineer_root/.cache/vision_${ROS_DOMAIN_ID:-0}.lock"
  if ! flock -n "$vision_lock_fd"; then
    echo "本工作区 ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0} 的视觉流程已经在运行，请先在原终端按 Ctrl+C；本次未启动节点。" >&2
    exit 1
  fi
fi

if [[ ! -f "$engineer_root/ros2_ws/install/setup.bash" ]]; then
  echo "请先运行 ./build.sh" >&2
  exit 1
fi
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
source "$engineer_root/ros2_ws/install/setup.bash"

pids=()
cleanup_started=false
wait_for_groups() {
  local shutdown_deadline=$((SECONDS + $1))
  local active pid
  while (( SECONDS < shutdown_deadline )); do
    active=false
    for pid in "${pids[@]}"; do
      if kill -0 -- "-$pid" 2>/dev/null; then
        active=true
      fi
    done
    if [[ "$active" == false ]]; then
      return 0
    fi
    sleep 0.1
  done
  return 1
}

cleanup() {
  if [[ "$cleanup_started" == true ]]; then
    return
  fi
  cleanup_started=true
  trap '' INT TERM
  # 每个 launch 使用独立进程组；终端 Ctrl+C 由这里统一转发，避免重复信号。
  # ros2 launch 收到 SIGINT 后负责关闭它启动的节点。
  for pid in "${pids[@]}"; do
    if kill -0 "$pid" 2>/dev/null; then
      kill -INT "$pid" 2>/dev/null || true
    fi
  done
  if ! wait_for_groups 12; then
    echo "部分启动进程未及时退出，向所属进程组发送 SIGTERM" >&2
    for pid in "${pids[@]}"; do
      kill -TERM -- "-$pid" 2>/dev/null || true
    done
    if ! wait_for_groups 4; then
      for pid in "${pids[@]}"; do
        kill -KILL -- "-$pid" 2>/dev/null || true
      done
    fi
  fi
  for pid in "${pids[@]}"; do
    wait "$pid" 2>/dev/null || true
  done
}
trap cleanup EXIT
trap 'exit 0' INT TERM

start_launch() {
  echo "启动：$*"
  # Bash 后台任务默认忽略 SIGINT；进入 Python 前恢复，才能响应统一退出。
  setsid env --default-signal=INT,QUIT,TERM /usr/bin/python3 /opt/ros/humble/bin/ros2 launch "$@" &
  pids+=("$!")
}

# 各模式的启动列表只在这里维护，不把视觉实验混入默认机械臂系统。
if [[ "$mode" == vision ]]; then
  if [[ "$start_camera" == true ]]; then
    start_launch camera_driver camera_driver.launch.py "${camera_arguments[@]}"
  fi
  if [[ "$start_detector" == true ]]; then
    start_launch yolo_detector yolo_detector.launch.py "${detector_arguments[@]}"
    if [[ "$start_icp" == true ]]; then
      start_launch model_cloud_publisher model_cloud_publisher.launch.py "${model_arguments[@]}"
      start_launch icp_registration icp_registration.launch.py "${icp_arguments[@]}"
    fi
  fi
  if [[ "$start_bridge" == true ]]; then
    start_launch camera_driver foxglove_bridge.launch.py "${bridge_arguments[@]}"
  fi
  start_launch video_recorder video_recorder.launch.py "${recorder_arguments[@]}"
  if [[ "$vision_use_rviz" == true ]]; then
    start_launch camera_driver vision_rviz.launch.py
  fi
else
  start_launch sim_controller sim_controller.launch.py "${controller_arguments[@]}"
  start_launch moveit_config moveit.launch.py
  start_launch engineer_mtc engineer_mtc.launch.py "${mtc_arguments[@]}"
fi

# 任一被监控的 launch 进程退出时，停止其余进程；正常用户退出由 trap 处理。
set +e
wait -n "${pids[@]}"
child_status=$?
set -e
if (( child_status == 0 )); then
  child_status=1
fi
echo "ROS launch 进程已退出，停止整套系统（状态码 $child_status）" >&2
exit "$child_status"
