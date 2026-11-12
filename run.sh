#!/usr/bin/env bash
# 总启动只在这里组织：各功能包的 launch 自行读取所属 YAML。
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'HELP'
用法：./run.sh [mode:=planning|vision] [use_rviz:=true|false]
               [start_planner:=true|false] [start_goal:=true|false]
               [controller_config:=FILE] [planner_config:=FILE] [goal_config:=FILE]
               [start_camera:=true|false] [start_bridge:=true|false]
               [camera_config:=FILE] [vision_config:=FILE] [foxglove_config:=FILE]
默认 planning：启动模拟控制、MoveIt、规划服务、Topic 客户端和 RViz。
vision：只启动 D435、main_visualizer 和 Foxglove Bridge；请先关闭 RealSense Viewer。
Foxglove 连接 ws://127.0.0.1:8765，use_rviz 参数仅用于 planning 模式。
各包 launch 独立加载默认 YAML，配置路径参数仅用于显式覆盖；Ctrl+C 统一退出。
HELP
}

mode=planning
use_rviz=true
start_planner=true
start_goal=true
controller_arguments=()
planner_arguments=()
goal_arguments=()
vision_arguments=()
for argument in "$@"; do
  case "$argument" in
    -h|--help) usage; exit 0 ;;
    mode:=planning|mode:=vision) mode="${argument#*:=}" ;;
    use_rviz:=true|use_rviz:=false) use_rviz="${argument#*:=}" ;;
    start_planner:=true|start_planner:=false) start_planner="${argument#*:=}" ;;
    start_goal:=true|start_goal:=false) start_goal="${argument#*:=}" ;;
    start_camera:=true|start_camera:=false|start_bridge:=true|start_bridge:=false)
      vision_arguments+=("$argument") ;;
    controller_config:=?*) controller_arguments=("$argument") ;;
    planner_config:=?*) planner_arguments=("$argument") ;;
    goal_config:=?*) goal_arguments=("$argument") ;;
    camera_config:=?*|vision_config:=?*|foxglove_config:=?*) vision_arguments+=("$argument") ;;
    *) echo "未知或无效参数：$argument" >&2; usage >&2; exit 2 ;;
  esac
done

# 同一工作区、同一 ROS 域只允许一套视觉流程，提前阻止相机和端口争用
if [[ "$mode" == vision ]]; then
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
  start_launch vision_nodes vision_pipeline.launch.py "${vision_arguments[@]}"
else
  start_launch sim_controller sim_controller.launch.py "${controller_arguments[@]}"
  start_launch moveit_config moveit.launch.py
  if [[ "$start_planner" == true ]]; then
    start_launch arm_planning arm_planning.launch.py "${planner_arguments[@]}"
  fi
  if [[ "$start_goal" == true ]]; then
    start_launch arm_goal arm_goal.launch.py "${goal_arguments[@]}"
  fi
  if [[ "$use_rviz" == true ]]; then
    start_launch arm_visualization rviz.launch.py
  fi
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
