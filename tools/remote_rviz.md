# 双机 MTC 预览：Jetson 规划，Ubuntu 22.04 电脑显示

两台机器使用 ROS 2 Humble，通信域为 `ROS_DOMAIN_ID=6`。电脑通过 ROS 2 DDS 直接接收数据，不通过 Foxglove Bridge 或 SSH 图形转发。

## 1. 电脑准备源码和显示环境

在电脑本地桌面终端执行。下列安装命令假定已配置 ROS 2 Humble 软件源。

```bash
sudo apt update
sudo apt install ros-humble-desktop ros-humble-moveit \
  ros-humble-moveit-task-constructor-visualization \
  ros-humble-rmw-fastrtps-cpp python3-colcon-common-extensions python3-yaml
```

电脑必须有工程源码和模型资源，不能直接使用 Jetson 的 ARM64 构建产物。可从远程仓库检出 `jetson` 分支，也可直接从 Jetson 同步最新本地源码：

```bash
mkdir -p ~/Engineer_27
rsync -av --exclude='/ros2_ws/build/' --exclude='/ros2_ws/install/' \
  --exclude='/ros2_ws/log/' --exclude='/.cache/' --exclude='/video/' \
  jetson@192.168.55.1:/home/jetson/Engineer_27/ ~/Engineer_27/
cd ~/Engineer_27
./build.sh --packages-select moveit_config sim_controller
```

`192.168.55.1` 是 Jetson USB 网络地址；通过局域网连接时换成 Jetson 实际地址。两台机器必须能双向通信，ROS 发现所需的 UDP 多播以及数据 UDP 通信须可达。仅有 SSH 或 Foxglove TCP 连接正常，不能证明 DDS 网络可用。

## 2. Jetson 启动规划端

在 Jetson 终端执行：

```bash
cd /home/jetson/Engineer_27
export ROS_LOCALHOST_ONLY=0
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
./build.sh --packages-up-to engineer_mtc sim_controller
./run.sh mode:=mtc use_rviz:=false
```

这保留工程原有硬件/模拟选择，只关闭 Jetson 的机械臂 RViz。实际选择仍由 `serial_driver/config/serial_driver.yaml` 的 `use_mock_hardware` 控制。视觉流程需要时在另一个终端运行 `./run.sh mode:=vision`。

## 3. 电脑仅启动 RViz

```bash
cd ~/Engineer_27
./run_rviz.sh
```

脚本加载本地模型及 MoveIt 参数，设置域 6、跨机器通信，并默认使用 Fast DDS。它只启动 RViz，不启动第二套控制器、robot_state_publisher、move_group 或 MTC。

默认界面包括当前机器人、规划场景、取矿和存车的 TCP 目标，以及单独显示的当前矿与槽内矿。MTC 面板使用 `/engineer_mtc/solution` 和同命名空间的 `description`、`statistics`。任务创建和规划后才会出现对应的信息。

## 4. 分别查看取矿与存车预览

在已启动 ROS 的 Jetson 另一个终端中执行：

```bash
cd /home/jetson/Engineer_27
source /opt/ros/humble/setup.bash
source ros2_ws/install/setup.bash
source tools/ros_env.sh
ros2 service call /engineer_mtc/plan std_srvs/srv/Trigger '{}'
```

`plan` 使用已有的新鲜视觉目标，规划成功后返回 READY，预览覆盖阶段 1–3。拔离完成后用 `plan_place` 独立规划存车。需要有效关节状态、视觉位姿和对应 TF。现有 `plan_manual` 接口可用于手动目标，本文不提供未经标定的目标位置。

在 **Motion Planning Tasks** 面板选择 `external pick`（阶段 1–3）或 `place held energy`（阶段 4–6），点击对应的解播放。MTC 解包含规划时的附着和释放场景。播放动画不需要调用 `/engineer_mtc/execute`，存车规划服务仍需要实际或模拟执行到拔离成功。

**Pick / Place TCP Targets** 显示当前请求使用的目标。取矿是橙色的 **Pregrasp → Grasp**，存车是青色的 **Pre-storage → Storage**。箭头只标示末端接近或入槽的直线方向。轨迹看 MTC 解或后备轨迹。新请求开始时清掉前一任务的目标，规划失败时可用已计算的目标排查，取消时清除目标。

默认关闭普通 **Trajectory Fallback** 显示，避免与 MTC 解动画重叠；需要单纯查看完整关节轨迹时，可关闭 **MTC Stages and Solution** 后开启它，话题为 `/engineer_mtc/display_planned_path`。普通轨迹显示不包含完整的阶段场景变化。

## 5. 电脑看不到节点时

在电脑另开终端：

```bash
source /opt/ros/humble/setup.bash
export ROS_DOMAIN_ID=6
export ROS_LOCALHOST_ONLY=0
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp
ros2 daemon stop
ros2 node list --no-daemon
ros2 topic echo /joint_states --once
```

应看到 Jetson 的节点，并能收到关节数据。若不行，先检查双机域 ID、RMW、网络接口、VPN、防火墙和多播。在电脑运行 `ros2 multicast receive`，Jetson 运行 `ros2 multicast send`，然后反向测试，可辅助判断发现通信；该测试通过仍需确认实际 ROS 数据可达。

若节点可见但模型缺失，检查电脑是否已构建并加载 `moveit_config` 及模型资源。若 MTC 面板插件加载失败，确认安装了 Humble 对应的 `moveit_task_constructor_visualization`。不要复制 ARM64 的 `install/` 到 x86_64 电脑。

双机 DDS 连通性需在实际使用的电脑与 Jetson 上按上述步骤验证。
