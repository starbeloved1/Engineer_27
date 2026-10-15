#!/usr/bin/env bash
# 总启动只在这里组织：各功能包的 launch 自行读取所属 YAML。
set -eo pipefail
engineer_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'HELP'
用法：./run.sh [use_rviz:=true|false] [start_planner:=true|false] [start_goal:=true|false]
               [controller_config:=FILE] [planner_config:=FILE] [goal_config:=FILE]
默认启动模拟控制、MoveIt、规划服务、Topic 客户端和 RViz。
各包 launch 独立加载默认 YAML，配置路径参数仅用于显式覆盖；Ctrl+C 统一退出。
HELP
}

use_rviz=true
start_planner=true
start_goal=true
controller_arguments=()
planner_arguments=()
goal_arguments=()
for argument in "$@"; do
  case "$argument" in
    -h|--help) usage; exit 0 ;;
    use_rviz:=true|use_rviz:=false) use_rviz="${argument#*:=}" ;;
    start_planner:=true|start_planner:=false) start_planner="${argument#*:=}" ;;
    start_goal:=true|start_goal:=false) start_goal="${argument#*:=}" ;;
    controller_config:=?*) controller_arguments=("$argument") ;;
    planner_config:=?*) planner_arguments=("$argument") ;;
    goal_config:=?*) goal_arguments=("$argument") ;;
    *) echo "未知或无效参数：$argument" >&2; usage >&2; exit 2 ;;
  esac
done

if [[ ! -f "$engineer_root/.cache/ros2/install/setup.bash" ]]; then
  echo "请先运行 ./scripts/build_ros.sh" >&2
  exit 1
fi
unset AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH PYTHONPATH LD_LIBRARY_PATH
source /opt/ros/humble/setup.bash
source "$engineer_root/.cache/ros2/install/setup.bash"

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

# 与 heros 一样，各功能包直接列在总脚本中，没有总 launch 包。
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
