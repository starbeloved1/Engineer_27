# Engineer 27：MTC 任务规划与取矿视觉

取矿视觉统一使用 PCL，接入 D435、YOLO/固定手动 ROI、框内点云、ICP、位姿后处理及
Foxglove/RViz 显示。MTC 将物体位姿转换为工具与末端目标，规划预抓取及可选直线接近，
发布轨迹预览。真实定位精度、相机安装 TF、TCP 和抓取偏移仍需实物验证。
当前 MTC 不执行运动，尚无夹爪开合、物体附着、抬起和搬运阶段。

## 构建与默认启动

在 `Engineer_27` 根目录，使用未激活 Conda 的系统 ROS 2 Humble 终端：

```bash
./build.sh
./run.sh
```

默认模式是 `mtc`，等同于 `./run.sh mode:=mtc`，启动：

```text
run.sh
  ├── sim_controller/sim_controller.launch.py  模拟控制、反馈、TF 和可选 RViz
  ├── moveit_config/moveit.launch.py           MoveIt 服务和规划场景
  └── engineer_mtc/engineer_mtc.launch.py       任务规划节点
```

机械臂显示由 `ros2_ws/src/sim_controller/config/sim_controller.yaml` 控制：

```yaml
use_rviz: true # false 关闭显示，保留模拟控制、反馈与 TF
```

修改配置后重新启动。`./run.sh use_rviz:=false` 可临时覆盖；不传此参数时使用 YAML。
这个开关只影响机械臂窗口，`mode:=vision` 的相机点云窗口仍由视觉模式的 `use_rviz` 参数控制。
Ctrl+C 统一退出，任一被监控的 launch 退出时停止其余流程。不要在同一 ROS 域重复启动机械臂系统。

`build.sh` 使用标准 `ros2_ws/build`、`install`、`log` 目录。`run.sh` 自动加载本工作区环境；
其他终端中的 ROS 命令需要先执行 `source ros2_ws/install/setup.bash`。
机械臂与基础 ROS 节点使用系统 Humble；YOLO 使用 Conda `engineer` 的 Python。

## MTC 小实验与数据流

另开终端，在 `Engineer_27` 根目录：

```bash
source ros2_ws/install/setup.bash
ros2 run engineer_mtc demo_pregrasp
```

示例用已知关节目标的 FK 反推虚拟矿位姿，发布到独立的手动输入，再调用规划服务。
它检查接口与几何计算，不代表实际相机标定或定位精度。示例的直线接近是否可行，取决于当前
`include_approach`、接近距离和工具偏移；只检查预抓取时可用覆盖文件设置 `include_approach: false`。

```text
视觉 PoseArray / 手动 PoseStamped
  → 缓存最新输入
  → 调用一次规划服务
  → 选择并固定目标，检查目标与关节反馈时效
  → 按观测时间查询 TF，转换到 base_link
  → 计算抓取 TCP 和预抓取 link_6 目标
  → 构造 MTC 任务并求解
  → 返回本次结果，成功时发布轨迹预览
```

| 接口 | 类型 | 作用 |
| --- | --- | --- |
| `/vision/icp/object_poses` | `geometry_msgs/PoseArray` | 视觉物体位姿输入 |
| `/engineer_mtc/manual_object_pose` | `geometry_msgs/PoseStamped` | 独立手动输入 |
| `/engineer_mtc/plan` | `std_srvs/Trigger` | 以视觉缓存触发规划 |
| `/engineer_mtc/plan_manual` | `std_srvs/Trigger` | 以手动缓存触发规划 |
| `/engineer_mtc/object_pose_base` | `PoseStamped` | 基座中的物体原点 |
| `/engineer_mtc/grasp_tcp` | `PoseStamped` | 基座中的抓取 TCP |
| `/engineer_mtc/pregrasp_pose` | `PoseStamped` | 预抓取 link_6 目标 |
| `/engineer_mtc/display_planned_path` | `moveit_msgs/DisplayTrajectory` | 最近一次成功规划的预览 |

持续输入不会自动触发规划，服务忙时拒绝新请求。三个目标在轨迹求解前发布；新请求失败时
旧轨迹可能仍然显示，应以本次服务的 `success/message` 判断结果。
真实视觉接入还需要相机到基座的 TF，以及正确的抓取和 TCP 偏移。

## 先运行相机与点云

关闭占用相机的 RealSense Viewer 或旧流程，在未激活 Conda 的系统终端运行：

```bash
cd /home/starbeloved/code/Robomaster/Engineer/Engineer_27
./build.sh
./run.sh mode:=vision use_rviz:=true
```

`vision_nodes/` 按 heros 的 `hero_aim/` 组织为功能分组：各节点独立成 ROS 包，
各自拥有构建文件、配置和 launch，共用计算导出为 `vision_core`。
`run.sh mode:=vision` 分别启动这些包；`mode:=vision_pcl` 保留为兼容别名。

相机使用 USB 3：彩色 1280×720、深度 848×480，均请求 30 FPS，算法实际处理频率另计。
权重 `model/best.pt` 和 PCL 模型 `assets/energy/energy.ply` 复制自 `engineer_2026/jetson`。
手动 ROI 开关在 `yolo_detector/config/yolo_detector.yaml`，RGB 原图录像开关在
`camera_driver/config/camera.yaml`，视频保存到 `video/`；路径均相对于视觉分组目录。

Foxglove 连接 `ws://127.0.0.1:8765`，Image 选择 YOLO 发布的 `/vision/visualization/image`；关闭检测时选择相机原图 `/camera/camera/color/image_raw`。
点云显示参考系选择 `camera_link`：`/vision/icp/target_cloud` 是观测，
`/vision/icp/raw_aligned_cloud` 是原始 ICP 模型，`/vision/icp/aligned_cloud` 是经过约束的最终模型。
主要业务输出是 `/vision/icp/object_poses`（PoseArray），不是夹爪 TCP 目标。

已启动视觉流程时，另开终端加载本工作区后执行
`ros2 launch camera_driver vision_rviz.launch.py`，只附加显示。
详细流程、参数和验证见 [PCL 视觉说明](ros2_ws/src/vision_nodes/README.md)。
此前的科技核心单目兑矿分析保留在 [北理 PnP 结构解析](docs/bit_pnp_architecture.md)。

## 各包的职责

| 包 | 职责 |
| --- | --- |
| `engineer_mtc` | 物体位姿到工具目标的转换、MTC 任务规划和预览 |
| `moveit_config` | 机器人模型、SRDF、KDL、OMPL、关节限制和控制器映射 |
| `sim_controller` | 模拟硬件、控制器、关节状态、机器人 TF 与可选 RViz 显示 |
| `vision_core` | PCL 公共计算与位姿约束库 |
| `camera_driver` | D435 官方驱动配置、Foxglove Bridge、视觉 RViz |
| `yolo_detector` | YOLO/固定手动 ROI、检测框和同帧叠框图像 |
| `model_cloud_publisher` | PLY 模型点云发布 |
| `icp_registration` | 框内点云、PCL ICP、位姿后处理和输出 |
| `video_recorder` | RGB 原图录像 |

`vision_nodes/` 是功能分组目录，其下各节点是独立 ROS 包。各包 launch 加载所属 YAML，
根目录 `run.sh` 维护启动列表，没有总 launch 包。

## 配置、模型与显示

| 配置位置 | 作用 |
| --- | --- |
| `engineer_mtc/config/engineer_mtc.yaml` | 输入、目标时效、抓取/TCP 偏移、接近参数与规划预算 |
| `sim_controller/config/sim_controller.yaml` | 启动显示开关 `use_rviz` |
| `sim_controller/config/mtc.rviz` | MTC 目标坐标轴与轨迹显示 |
| `moveit_config/config/arm.srdf` | 规划组、零位与相邻碰撞豁免 |
| `moveit_config/config/kinematics.yaml` | KDL 数值 IK 配置 |
| `moveit_config/config/ompl_planning.yaml` | RRTConnect 与规划管线配置 |
| `moveit_config/config/joint_limits.yaml` | 离线速度和加速度限制 |
| `moveit_config/config/moveit_controllers.yaml` | MoveIt 控制器映射 |
| `sim_controller/config/ros2_controllers.yaml` | 模拟控制器与关节接口 |

路径相对于 `ros2_ws/src/`。业务参数启动时读取，修改后重启对应节点。
可使用 `controller_config:=FILE`、`sim_config:=FILE`、`mtc_config:=FILE` 覆盖配置；视觉参数见视觉分组说明。
例如部分覆盖文件：

```yaml
engineer_mtc:
  ros__parameters:
    include_approach: false
```

```bash
./run.sh mtc_config:=/absolute/path/override.yaml
```

`assets/engineer/arm.urdf` 与 `assets/engineer/meshes/{visual,collision}` 是运行模型来源；构建时仅将网格路径
转换为安装用 `package://moveit_config/...` 路径，不修改原模型或 `assets/source/` 归档。
当前规划末端为 `link_6` 原点，尚不是已标定的夹爪 TCP。

单独附加 MTC 显示：`ros2 launch sim_controller rviz.launch.py`。
可选原生调试窗口：`ros2 launch sim_controller native_rviz.launch.py`，直接访问 MoveIt，独立于 MTC 任务。
这两个显式显示入口只打开窗口，不启动模拟硬件、控制器、状态发布器或规划服务；
真机接入后也可使用，模型状态来自外部提供的实际关节反馈和 TF。

`sim_controller/sim_controller/launch_support.py` 为本机 Humble 的 RViz 进程预加载显示插件，
`moveit_config/moveit_config/configuration.py` 为 move_group 预加载相关插件，避免退出时插件卸载
早于 ROS 回调对象析构；兼容处理仅作用于对应进程，不修改系统库。

## 为什么有两份 controller 配置

- `moveit_config/config/moveit_controllers.yaml` 由 MoveIt 读取：描述下游控制器名称、关节与 FollowJointTrajectory Action 映射，告诉 MoveIt“轨迹交给谁”。它不创建 ros2_control 控制器。
- `sim_controller/config/ros2_controllers.yaml` 由 controller_manager 和控制器读取：定义实际插件、六个关节、position 命令接口、状态接口和控制频率。
- `sim_controller/config/arm.ros2_control.xacro` 在纯模型外添加 GenericSystem 模拟硬件接口。`moveit_config/config/arm.urdf.xacro` 只包含纯模型，不再包含模拟硬件。

两份 YAML 中的控制器名和关节名必须匹配。今后更换执行端时，修改对应执行包和 MoveIt 的映射；规划算法不需要迁入控制包。

## 当前边界与验证

机器人模型仍保留 6 个 continuous 关节，角度限位和速度能力尚未按实物核定。
MTC 使用已有场景进行规划，不自动将视觉点云或矿位姿加入碰撞场景。
规划成功只表示当前模型和场景下找到方案，不表示真实抓取成功。

代码阅读顺序：`engineer_mtc/src/mtc_node.cpp::plan()` → `grasp_geometry.cpp` →
`pregrasp_task.cpp`。头文件声明数据结构，launch 负责加载机器人配置和业务参数。

总脚本进程管理回归使用独立 ROS 域，无需预先启动系统：

```bash
ROS_DOMAIN_ID=195 /usr/bin/python3 scripts/test_run_supervision.py
```

它检查 YAML 关闭显示、命令行覆盖开关、默认 MTC 启动后的 Ctrl+C 清理、子 launch 退出联动清理、
非法参数拒绝，以及视觉重复启动的互斥。视觉测试关闭相机、检测和 Bridge，不占用相机。MTC 小实验用于验证规划与预览链路。

## 模型资源与独立预览

```text
assets/
├── engineer/   # 机械臂 URDF、visual、collision
├── energy/     # 单块能量单元 URDF、STL、PLY 和碰撞覆盖报告
└── source/     # 原始资料归档
model/          # YOLO 权重
tools/         # 独立预览与能量单元 URDF 生成工具
```

预览入口已从 assets 移到 `tools/preview.launch.py`。使用方法见 [tools](tools/README.md)。
能量单元为新建的单块描述，碰撞采用两端圆柱和中部方盒，保留抓取窄腰；
[模型说明和 26 对比](assets/energy/README.md) 包含尺寸、坐标约定和覆盖校验。
它目前作为独立资源安装，尚未自动接入 MTC 场景或标定到 ICP 模型坐标系。
