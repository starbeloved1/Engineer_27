# Engineer 27：离线路径规划

当前系统使用现有模型，完成关节目标和末端位姿目标规划、碰撞检查、轨迹预览及模拟执行。业务节点和目标客户端均为 C++；Python 用于 launch、资源转换与测试。视觉、真实控制接口、TCP 标定和实际机械限位不在本阶段范围。

## 六个包的职责

| 包 | 职责 | 运行节点或组件 |
| --- | --- | --- |
| `eng_interfaces` | 公共 `PlanMotion.action` 和 Topic 目标、反馈、结果消息 | 不运行节点 |
| `arm_goal` | C++ 命令行和 Topic 目标输入，统一转发到规划 Action | 临时 `/arm_plan_goal` 或常驻 `/arm_goal` |
| `arm_planning` | C++ 规划服务、IK 筛选、路径复查和规划计算 | `/arm_planning` |
| `moveit_config` | 纯机器人模型、SRDF、KDL、OMPL、MoveIt 执行端映射 | launch 启动 `/move_group` |
| `sim_controller` | 模拟硬件 Xacro、ros2_control 配置、控制器和关节状态发布 | `/controller_manager`、`arm_controller`、`joint_state_broadcaster`、`robot_state_publisher` |
| `eng_bringup` | 组合各功能包 launch，维护 world→base_link 和 RViz 显示 | 启动组合，不另建业务节点 |

每个功能包都有自己的 launch，加载所属 YAML；`eng_bringup/launch/offline.launch.py` 使用 IncludeLaunchDescription 组合它们。根目录 `run.sh` 加载本工作区环境并进入同一总启动，不维护第二套节点列表。

```text
run.sh → eng_bringup/offline.launch.py
  ├── sim_controller/sim_controller.launch.py  模拟硬件、控制器和状态发布
  ├── moveit_config/moveit.launch.py           MoveIt 服务
  ├── arm_planning/arm_planning.launch.py      自研 Action 服务端
  ├── arm_goal/arm_goal.launch.py              常驻 Topic→Action 客户端
  └── eng_bringup/rviz.launch.py               只显示的 RViz
```

`assets/arm.urdf` 与 `assets/meshes/{visual,collision}` 是模型的唯一来源。构建时配置包生成安装用 URDF，仅将网格相对路径改为 `package://moveit_config/...`；不修改源 URDF，不安装 `assets/source` 归档。没有独立 description 包。

离线网格处理工具仍放在 `scripts/`，其 Conda 环境由 `environment.yml` 管理；ROS 运行不使用该环境。

## 构建和启动

在系统 ROS 2 Humble 环境中操作，退出已激活的 Conda 环境。先关闭旧的 preview 或其他同域机械臂实例，避免关节反馈与控制器冲突。

```bash
cd /home/starbeloved/code/Robomaster/Engineer_27
./scripts/build_ros.sh
source .cache/ros2/install/setup.bash
./run.sh
```

构建脚本只加载系统 ROS 并使用全新的 `.cache/ros2/{build,install,log}` 目录；以后继续使用这一目录。仓库中已跟踪的历史 `ros2_ws/build`、`install`、`log` 本次不清理，也不作为运行环境。

直接使用 ROS 命令的等价入口是 `ros2 launch eng_bringup offline.launch.py`。客户端已迁到 `arm_goal`，命令改为 `ros2 run arm_goal plan_goal`。规划 Action 仍为 `/arm_planning/plan_motion`，类型仍为 `eng_interfaces/action/PlanMotion`，字段未改。

默认 RViz **只显示**当前模型、规划轨迹和场景，没有原生 Plan / Execute 按钮。`Current Robot` 来自当前关节反馈的 TF，橙色 `Validated Trajectory` 是复查后的预览；预览不改变当前关节状态。无需窗口时：

```bash
ros2 launch eng_bringup offline.launch.py use_rviz:=false
```

附加默认显示窗口使用 `ros2 launch eng_bringup rviz.launch.py`。单独调试 MoveIt 原生功能时，使用 `ros2 launch eng_bringup native_rviz.launch.py`；这个窗口直接访问 MoveIt，**不经过自研节点的 IK 候选选择及轨迹复查**，不作为默认业务入口。两种附加窗口均不会重复启动规划服务或控制器。

本机 Humble 的独立显示插件曾在 RViz 退出时触发共享 ROS node 析构崩溃。`eng_bringup/eng_bringup/launch_support.py` 统一为 RViz 预加载相关插件，使库在节点清理期间保持加载；只作用于 RViz 进程，不修改系统库或规划节点。

本机 Humble 的 move_group 也曾在退出插件卸载阶段触发 CallbackGroup 析构崩溃。`moveit_config/moveit_config/configuration.py` 为 move_group 进程预加载默认能力和控制器管理插件，保持它们加载到进程退出；此兼容配置不修改系统库。

## 提交目标：C++ 命令行客户端

另开终端并加载相同 ROS 和工作区环境。客户端运行期间存在节点 `/arm_plan_goal`，收到结果后退出。

```bash
# 仅规划和显示，不改变当前关节反馈
ros2 run arm_goal plan_goal --joints 0.25 0 0 0 0 0

# 规划并执行到模拟控制器
ros2 run arm_goal plan_goal --joints 0.25 0 0 0 0 0 --execute

# 模拟回到零位，同时导出完整轨迹 JSON
ros2 run arm_goal plan_goal --joints 0 0 0 0 0 0 --execute --output /tmp/home_plan.json

# 当前模型中由 FK 得到的末端位姿示例，默认参考系 base_link
ros2 run arm_goal plan_goal --pose 0.0534446 0.00930325 0.19012849 -0.00996671 0.09933467 0.09933467 0.99003329
```

`--joints` 顺序为 `link_1_joint` 至 `link_6_joint`，单位 rad。`--pose` 顺序为 `X Y Z QX QY QZ QW`，位置单位 m，朝向为单位四元数；`--frame` 支持 `base_link` 或 `world`。目前规划末端是 `link_6` 原点，不是已标定的夹持中心 TCP。

可选参数：`--planning-time`、`--velocity-scale`、`--acceleration-scale`、`--timeout` 和 `--output`。未指定的速度、加速度缩放及规划时间以 0 发送，由服务端 YAML 决定默认值（0.2、0.2、5 s）。请求中的非零值优先；规划时间范围 `[0,30]`，缩放范围 `[0,1]`。规划时限只约束 OMPL 搜索，客户端结果超时覆盖整个请求。

客户端会打印阶段反馈和结果摘要。JSON 导出保持 `success`、`error_code`、`message`、`selected_joint_positions`、规划耗时、路径长度、复查数、IK 候选数、关节名称和全部轨迹点字段。退出码为：成功 0、规划或执行失败 1、输入或通信错误 2、Ctrl+C 取消 130。Ctrl+C 会先请求取消服务端任务，再关闭客户端 ROS 上下文。

## 提交目标：常驻 Topic 客户端

总启动默认启动 `/arm_goal`。也可用 `start_goal:=false` 关闭，并通过 `ros2 launch arm_goal arm_goal.launch.py` 或 `ros2 run arm_goal plan_goal --listen` 单独启动。不要同时启动两个相同的 Topic 客户端，避免重复转发目标。

```text
其他节点 → /arm_goal/target → arm_goal → /arm_planning/plan_motion → arm_planning
其他节点 ← /arm_goal/feedback ← arm_goal ← Action Feedback
其他节点 ← /arm_goal/result   ← arm_goal ← Action Result
其他节点 → /arm_goal/cancel   → arm_goal → Action 取消请求
```

| Topic（默认） | 类型 | 方向与含义 |
| --- | --- | --- |
| `/arm_goal/target` | `eng_interfaces/msg/MotionTarget` | 输入；request_id 和完整规划目标 |
| `/arm_goal/cancel` | `std_msgs/msg/String` | 输入；data 是要取消的当前 request_id |
| `/arm_goal/feedback` | `eng_interfaces/msg/MotionFeedback` | 输出；request_id、stage、detail |
| `/arm_goal/result` | `eng_interfaces/msg/MotionResult` | 输出；request_id 和完整 Action 结果，包括轨迹 |

`MotionTarget` 支持 JOINT=0、POSE=1；其余目标字段与 PlanMotion Goal 对应，单位、坐标系和默认值相同。`request_id` 必须非空，每次新任务使用不同编号。`execute` 默认 false，设 true 时请求模拟执行。反馈中的 `accepted` 只表示 Topic 客户端接收了目标，服务端仍可能拒绝。

目标使用 Reliable + Volatile QoS，订阅者不会获得启动前的历史命令。一次只转发一个任务；忙时新编号的目标返回失败，不排队、不抢占，当前任务同编号的重复目标被忽略。CLI 和 Topic 模式共用 `GoalClientNode::submit()`，服务端也只允许一个任务；CLI 与 Topic 同时提交时，其中一个可能被拒绝。输入或通信失败会发布结果，常驻节点继续等待后续目标。结果与反馈也使用 Volatile，需先订阅再发送任务。

先在观察终端订阅：

```bash
ros2 topic echo /arm_goal/feedback
# 另开终端观察最终消息；完整 MotionResult 还包含轨迹等字段
ros2 topic echo /arm_goal/result --field message
```

发送一次关节目标：

```bash
ros2 topic pub --once /arm_goal/target eng_interfaces/msg/MotionTarget \
  "{request_id: joint_demo_1, target_type: 0, joint_positions: [0.25, 0.0, 0.0, 0.0, 0.0, 0.0], execute: false}"
```

发送一次末端位姿目标：

```bash
ros2 topic pub --once /arm_goal/target eng_interfaces/msg/MotionTarget \
  "{request_id: pose_demo_1, target_type: 1, target_pose: {header: {frame_id: base_link}, pose: {position: {x: 0.0534446, y: 0.00930325, z: 0.19012849}, orientation: {x: -0.00996671, y: 0.09933467, z: 0.09933467, w: 0.99003329}}}, execute: false}"
```

取消仍在运行的对应任务：

```bash
ros2 topic pub --once /arm_goal/cancel std_msgs/msg/String "{data: joint_demo_1}"
```

客户端结果等待超时会请求取消；收到匹配编号的取消消息或 Ctrl+C 时也会取消已接收的 Action 任务。取消请求与任务恰好完成可能同时发生，最终以服务端状态和实际反馈为准。

## 为什么有两份 controller 配置

- `moveit_config/config/moveit_controllers.yaml` 由 MoveIt 读取：描述下游控制器名称、关节与 FollowJointTrajectory Action 映射，告诉 MoveIt“轨迹交给谁”。它不创建 ros2_control 控制器。
- `sim_controller/config/ros2_controllers.yaml` 由 controller_manager 和控制器读取：定义实际插件、六个关节、position 命令接口、状态接口和控制频率。
- `sim_controller/config/arm.ros2_control.xacro` 在纯模型外添加 GenericSystem 模拟硬件接口。`moveit_config/config/arm.urdf.xacro` 只包含纯模型，不再包含模拟硬件。

两份 YAML 中的控制器名和关节名必须匹配。今后更换执行端时，修改对应执行包和 MoveIt 的映射；规划算法不需要迁入控制包。

## 配置归属与覆盖

| 文件（相对于对应包） | 实际使用方 | 内容 |
| --- | --- | --- |
| `arm_planning/config/arm_planning.yaml` | `/arm_planning` | 自定义 Action 地址、场景服务、预览话题、规划组、末端、规划器、参考系、IK 与复查参数、默认缩放和规划时限 |
| `arm_goal/config/plan_goal.yaml` | `/arm_plan_goal` | 服务端 Action 地址、首次发现后的匹配等待、发现服务端超时、目标接收超时、结果超时 |
| `arm_goal/config/arm_goal.yaml` | `/arm_goal` | 目标、取消、反馈、结果 Topic 地址，Action 地址、首次匹配等待及通信超时 |
| `moveit_config/config/arm.srdf` | MoveIt 模型 | 规划组、零位、虚拟关节、相邻碰撞豁免 |
| `moveit_config/config/kinematics.yaml` | KDL 插件 | 数值 IK 求解配置 |
| `moveit_config/config/ompl_planning.yaml` | OMPL 插件 | RRTConnect、搜索与碰撞离散检查配置 |
| `moveit_config/config/joint_limits.yaml` | MoveIt 时间参数化 | 离线速度与加速度上限 |
| `moveit_config/config/moveit_controllers.yaml` | MoveIt 执行管理器 | 轨迹控制器名称与 Action 映射 |
| `sim_controller/config/ros2_controllers.yaml` | `/controller_manager` 及控制器 | 模拟控制器类型、关节顺序、接口与更新频率 |
| `eng_bringup/config/display.rviz` | 默认 RViz | 仅状态、轨迹、场景显示 |
| `eng_bringup/config/native_moveit.rviz` | 可选原生 RViz | 直接调用 MoveIt 的调试面板 |

规划服务端、CLI 客户端和常驻 Topic 客户端均自动加载各自已安装的默认 YAML，后传入的 ROS 参数可覆盖默认值。客户端的 `--timeout` 另外覆盖结果等待时间。`discovery_settle_time` 默认 0.5 s：首次发现 Action 服务端后留出响应通道匹配时间，缓解本机 Fast DDS 短时客户端的接收响应超时；0 可禁用，它不计入结果等待时限。业务参数在启动时读取，修改文件后需重启对应进程。

例如给服务端使用一个参数覆盖文件：

```yaml
arm_planning:
  ros__parameters:
    planning_action: /experiment/plan_motion
    display_topic: /experiment/validated_path
    ik_attempts: 8
```

```bash
ros2 launch eng_bringup offline.launch.py planner_config:=/tmp/planner_override.yaml
ros2 run arm_goal plan_goal --joints 0.25 0 0 0 0 0 --ros-args -p planning_action:=/experiment/plan_motion
```

默认显示订阅 `/arm_planning/display_planned_path`，只接收自研节点复查后发布的轨迹。MoveIt 的 `/display_planned_path` 留给原生调试面板。如果改变 `display_topic`，也要在 RViz 的 Validated Trajectory 显示项同步修改订阅话题。MoveIt 标准 Action、关节反馈、TF 等保持其标准名称，需要改名时使用原生配置或 ROS remapping。

## 节点与通信地图

```text
arm_plan_goal（C++ 临时 CLI 客户端）或 arm_goal（C++ 常驻 Topic 客户端）
  └─ PlanMotion Action → arm_planning（C++ 业务服务）
       ├─ KDL：本进程 IK 候选求解与筛选
       ├─ GetPlanningScene Service → move_group
       ├─ MoveGroup Action → move_group → OMPL 规划与时间参数化
       ├─ 本进程轨迹插值碰撞复查
       ├─ DisplayTrajectory Topic → RViz
       └─ ExecuteTrajectory Action → move_group
             └─ FollowJointTrajectory Action → arm_controller → 模拟硬件

joint_state_broadcaster → /joint_states → move_group、arm_planning、robot_state_publisher
robot_state_publisher → /tf → RViz 等 TF 消费方
静态变换节点 → /tf_static：world 与 base_link 重合
```

| 节点／组件 | 接口名（默认） | 类型 | 数据方向 |
| --- | --- | --- | --- |
| `arm_plan_goal` / `arm_goal` ↔ `arm_planning` | `/arm_planning/plan_motion` | Action：`eng_interfaces/action/PlanMotion` | 目标向服务端；反馈与结果回客户端 |
| `arm_planning` ↔ `move_group` | `/get_planning_scene` | Service：`moveit_msgs/srv/GetPlanningScene` | 请求读取最新场景 |
| `arm_planning` ↔ `move_group` | `/move_action` | Action：`moveit_msgs/action/MoveGroup` | 请求路径规划，返回轨迹 |
| `arm_planning` ↔ `move_group` | `/execute_trajectory` | Action：`moveit_msgs/action/ExecuteTrajectory` | 请求执行已校验轨迹 |
| `move_group` ↔ `arm_controller` | `/arm_controller/follow_joint_trajectory` | Action：`control_msgs/action/FollowJointTrajectory` | 轨迹命令及执行反馈 |
| `joint_state_broadcaster` → 状态消费者 | `/joint_states` | Topic：`sensor_msgs/msg/JointState` | 持续发布模拟关节状态 |
| `robot_state_publisher` → TF 消费者 | `/tf` | Topic：`tf2_msgs/msg/TFMessage` | 根据反馈计算动态连杆变换 |
| 静态变换节点 → TF 消费者 | `/tf_static` | Topic：`tf2_msgs/msg/TFMessage` | 固定 world → base_link 变换 |
| `arm_planning` → 默认 RViz | `/arm_planning/display_planned_path` | Topic：`moveit_msgs/msg/DisplayTrajectory` | 复查后的预览轨迹，不改变关节反馈 |
| MoveIt → 原生调试 RViz | `/display_planned_path` | Topic：`moveit_msgs/msg/DisplayTrajectory` | MoveIt 自身的预览轨迹 |
| `move_group` → RViz | `/monitored_planning_scene` | Topic：`moveit_msgs/msg/PlanningScene` | 规划场景更新 |
| `robot_state_publisher` → RViz | `/robot_description` | Topic：`std_msgs/msg/String` | 发布模型描述供显示加载 |

`MoveGroupInterface::plan()` 是进程内 C++ 调用，它封装对 `/move_action` 的 ROS 通信。KDL、OMPL、碰撞检查库不是每个都单独启动一个节点。YAML 配置通过参数传入节点，不是目标消息，也不是额外的通信节点。

观察实际运行情况：

```bash
ros2 node list
ros2 node info /arm_planning
ros2 action list -t
ros2 param get /arm_planning ik_attempts
ros2 topic info /arm_planning/display_planned_path --verbose
ros2 topic echo /joint_states --once
```

## 推荐的代码阅读顺序

1. `eng_interfaces/action/PlanMotion.action`：先明确输入、输出和阶段反馈
2. `arm_planning/config/arm_planning.yaml` 和 `arm_goal/config/*.yaml`：明确参数归属和通信地址
3. `arm_planning/src/main.cpp`、`arm_planning_node.cpp` 及对应头文件：节点初始化、接收目标、反馈、取消、单任务调度
4. `arm_planning/src/motion_planner.cpp`：从 `plan()` 看场景、IK、目标检查、MoveIt 调用、复查和执行；此层依赖 ROS/MoveIt
5. `arm_planning/src/planning_math.cpp`：角度跨圈、候选评分、插值细分，独立于 ROS/MoveIt
6. `arm_goal/src/plan_goal_main.cpp`、`goal_options.cpp`、`goal_client_node.cpp`、`topic_input.cpp`：CLI/Topic 输入与共用的 Action 提交逻辑
7. `eng_bringup/launch/offline.launch.py`：把配置和各节点组装起来

## 当前算法边界

- 保留 6 个 continuous 关节，尚未根据实物设置角度限位
- 离线速度上限 1 rad/s、加速度上限 1 rad/s²，默认再乘 0.2，尚未核定真机性能
- SRDF 只排除 6 对直接相邻连杆；其余连杆继续检测，豁免需结合实物复核
- `[0,0,0.1,0,0,0]` 是已知干涉目标，涉及 `link_1` 与 `link_4/link_5`，未通过扩大豁免表绕过
- KDL 有限种子预算中选附近有效解，不保证找全全部解，未加入奇异性评分；选中候选规划失败时不自动遍历其余候选
- 目标朝向约束最终状态，未要求全过程保持该朝向
- 轨迹复查是离散插值检查，不是连续碰撞证明；场景在复查后变化需要重新规划
- 模拟执行只更新 mock 硬件，没有真实设备通信

## 验证

纯计算与命令行解析单测：

```bash
cd /home/starbeloved/code/Robomaster/Engineer_27
source .cache/ros2/install/setup.bash
colcon --log-base .cache/ros2/log test --build-base .cache/ros2/build --packages-select arm_planning arm_goal --event-handlers console_direct+
colcon test-result --test-result-base .cache/ros2/build --verbose
```

八项集成测试针对独立 ROS 域内的模拟系统运行，会添加并移除测试障碍物、执行及取消模拟运动：

```bash
# 终端一：加载当前工作区
ROS_DOMAIN_ID=187 ros2 launch eng_bringup offline.launch.py use_rviz:=false

# 终端二：加载当前工作区，从 Engineer_27 目录运行
ROS_DOMAIN_ID=187 /usr/bin/python3 ros2_ws/src/arm_planning/test/offline_integration.py
```

集成测试覆盖已知干涉、连续关节角、关节路径、IK/FK 回验、不可达目标、输入校验、环境障碍物、执行与取消、忙时拒绝，以及起终点有效而直接插值发生干涉时的绕行规划。

安装后 C++ 客户端的端到端验证（同一个隔离域中，先完成上面的八项测试）：

```bash
ROS_DOMAIN_ID=187 /usr/bin/python3 ros2_ws/src/arm_goal/test/client_integration.py
```

它检查原生二进制、输入错误、YAML/ROS 参数覆盖、无服务端超时、两类目标、JSON 导出、只规划时状态不变、模拟执行、忙时拒绝、Ctrl+C 和请求超时取消。

Topic 客户端端到端验证，在上述测试结束后顺序运行：

```bash
ROS_DOMAIN_ID=187 /usr/bin/python3 ros2_ws/src/arm_goal/test/topic_integration.py
```

覆盖关节/位姿目标、反馈和完整结果、只规划时状态不变、碰撞与非法输入、模拟执行、忙时拒绝、活动目标去重、取消编号匹配、自定义 Topic/YAML、服务端不可用和超时取消。多组运动测试不要并行运行。
