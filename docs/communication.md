# Engineer_27：包、节点、接口与通信流程

总启动只在项目根目录 `run.sh` 中维护。包用于组织代码/配置，节点是运行中的通信参与者；launch 和 run.sh 负责启动程序，本身不是业务节点。

## 启动关系

```text
./run.sh
  ├── ros2 launch sim_controller sim_controller.launch.py
  ├── ros2 launch moveit_config moveit.launch.py
  ├── ros2 launch arm_planning arm_planning.launch.py
  ├── ros2 launch arm_goal arm_goal.launch.py
  └── ros2 launch arm_visualization rviz.launch.py
```

各包 launch 自己加载该包的 YAML。run.sh 记录各 launch 的 PID，按 Ctrl+C 时统一关闭；任一被监控的 launch 进程退出也会停止其他 launch。它监控的是 launch 进程，不是每个 ROS 节点的健康状态。`use_rviz:=false` 可关闭窗口，`start_goal:=false` 可关闭 Topic 客户端，`start_planner:=false` 可关闭自研服务端。

## 六个包的功能与对应节点

| 包 | 文件和职责 | 实际运行的节点或组件 | 主要接口 |
| --- | --- | --- | --- |
| `act_interfaces` | `PlanMotion.action`，`MotionTarget/Feedback/Result.msg`，只定义公共数据格式 | 无 | 生成 Action 和消息类型，不创建 Topic 或 Action 地址 |
| `arm_goal` | `plan_goal` C++ 程序；命令行解析、Topic 目标接收、Action 提交、反馈、取消、结果和 CLI JSON 输出 | CLI：临时 `/arm_plan_goal`；`--listen`：常驻 `/arm_goal` | 调用 `/arm_planning/plan_motion`；Topic 模式订阅 `/arm_goal/target`、`/arm_goal/cancel`，发布 `/arm_goal/feedback`、`/arm_goal/result` |
| `arm_planning` | 自研规划业务：校验输入、读取起点/场景、多种子 IK、候选筛选、请求 MoveIt、插值复查、预览与可选执行 | 程序 `arm_planning_node`，节点 `/arm_planning` | 提供 `/arm_planning/plan_motion`；请求 `/get_planning_scene`、`/move_action`、`/execute_trajectory`；发布 `/arm_planning/display_planned_path`；内部状态监视器订阅 `/joint_states` |
| `moveit_config` | 纯模型资源安装、SRDF、KDL、OMPL、速度/加速度限制、执行端映射；launch 启动 MoveIt | 主节点 `/move_group`；其内部可能创建 MoveIt 私有、TF 和执行管理辅助节点 | 提供规划、执行、场景服务；接收 `/joint_states` 和 TF；发布 `/monitored_planning_scene`；执行时调用控制器 Action |
| `sim_controller` | GenericSystem 模拟硬件 Xacro、ros2_control 参数、控制器加载、关节状态和 TF 发布 | `/controller_manager`；控制器 `arm_controller`、`joint_state_broadcaster`；`/robot_state_publisher`；`/world_to_base`；两个短时 spawner | 提供 `/arm_controller/follow_joint_trajectory`；发布 `/joint_states`、`/tf`、`/tf_static`、模型描述；controller_manager 提供加载/配置/切换/查询控制器的服务 |
| `arm_visualization` | RViz 配置和进程兼容设置，只负责默认显示或可选原生调试窗口 | 默认 `/rviz2`；原生调试入口 `/rviz_native`，插件还可能创建辅助节点 | 默认订阅 TF、模型、复查后的轨迹和场景；原生 MotionPlanning 插件可直接请求 MoveIt |

`move_group`、RViz、robot_state_publisher、controller_manager 及两种控制器的实现来自已安装的 MoveIt/ROS/ros2_control 软件。我们的包提供配置和启动方式，并没有重新实现这些标准组件。KDL、OMPL、碰撞检测是库/插件，不是这里额外启动的独立业务节点。

## YAML 的读取方

| 配置 | 谁读取 | 决定什么 |
| --- | --- | --- |
| `arm_goal/config/plan_goal.yaml` | `/arm_plan_goal` | Action 地址、首次发现后的匹配等待、发现/接收/结果超时 |
| `arm_goal/config/arm_goal.yaml` | `/arm_goal` | 四个 Topic 地址和上述 Action 通信配置 |
| `arm_planning/config/arm_planning.yaml` | `/arm_planning` | Action、场景 Service、显示 Topic 地址，以及规划组、末端、IK、复查与规划默认参数 |
| `moveit_config/config/kinematics.yaml` | 加载机器人运动学的进程 | KDL 插件和求解配置 |
| `moveit_config/config/ompl_planning.yaml` | MoveIt 规划流水线 | OMPL/RRTConnect 和搜索相关配置 |
| `moveit_config/config/joint_limits.yaml` | MoveIt | 离线速度和加速度上限；目前未据实物设置位置限位 |
| `moveit_config/config/moveit_controllers.yaml` | MoveIt 执行管理器 | 找哪个 FollowJointTrajectory Action、控制哪些关节；不创建控制器 |
| `sim_controller/config/ros2_controllers.yaml` | controller_manager 及控制器 | 实际加载的控制器插件、关节、命令/状态接口、更新频率 |
| `arm_visualization/config/*.rviz` | RViz | 显示插件、订阅话题、颜色和视角 |

YAML 是配置文件。代码读取地址后创建 ROS 通信对象；YAML 本身不会发送目标、运行规划或转发消息。

## 主要业务接口清单

以下省略节点自动生成的参数服务、日志话题和 Action 内部隐藏接口；实际完整清单可用 ros2 CLI 查询。

| 名称 | 机制和类型 | 方向 | 数据含义 |
| --- | --- | --- | --- |
| `/arm_goal/target` | Topic：`act_interfaces/msg/MotionTarget` | 目标发布者 → `/arm_goal` | request_id、JOINT/POSE 目标、是否执行和规划参数 |
| `/arm_goal/cancel` | Topic：`std_msgs/msg/String` | 目标发布者 → `/arm_goal` | data 是要取消的当前 request_id |
| `/arm_goal/feedback` | Topic：`act_interfaces/msg/MotionFeedback` | `/arm_goal` → 订阅者 | request_id、stage、detail |
| `/arm_goal/result` | Topic：`act_interfaces/msg/MotionResult` | `/arm_goal` → 订阅者 | request_id、成功/错误、轨迹、目标角及统计 |
| `/arm_planning/plan_motion` | Action：`act_interfaces/action/PlanMotion` | `/arm_goal` 或 `/arm_plan_goal` ↔ `/arm_planning` | 整个规划/可选执行任务，带反馈、结果和取消 |
| `/get_planning_scene` | Service：`moveit_msgs/srv/GetPlanningScene` | `/arm_planning` ↔ `/move_group` | 请求并返回场景快照 |
| `/move_action` | Action：`moveit_msgs/action/MoveGroup` | `/arm_planning` ↔ `/move_group` | 请求搜索路径，返回规划轨迹 |
| `/execute_trajectory` | Action：`moveit_msgs/action/ExecuteTrajectory` | `/arm_planning` ↔ `/move_group` | 执行已经取得的轨迹 |
| `/arm_controller/follow_joint_trajectory` | Action：`control_msgs/action/FollowJointTrajectory` | MoveIt ↔ `arm_controller` | 带时间的六轴轨迹和执行结果 |
| `/joint_states` | Topic：`sensor_msgs/msg/JointState` | `joint_state_broadcaster` → MoveIt、自研规划、robot_state_publisher | 当前关节名称、位置、速度等状态 |
| `/tf` | Topic：`tf2_msgs/msg/TFMessage` | robot_state_publisher → TF 消费者 | 由关节反馈计算的动态连杆变换 |
| `/tf_static` | Topic：`tf2_msgs/msg/TFMessage` | world_to_base、robot_state_publisher → TF 消费者 | world→base_link 及模型固定关节的静态变换 |
| `/robot_description` | Topic：`std_msgs/msg/String` | 模型发布方（robot_state_publisher、MoveIt）→ RViz 等 | URDF；主要节点也由各自 launch 直接获得模型参数 |
| `/arm_planning/display_planned_path` | Topic：`moveit_msgs/msg/DisplayTrajectory` | `/arm_planning` → 默认 RViz | 自研复查后的轨迹预览 |
| `/monitored_planning_scene` | Topic：`moveit_msgs/msg/PlanningScene` | `/move_group` → RViz 等 | 规划场景更新 |
| `/display_planned_path` | Topic：`moveit_msgs/msg/DisplayTrajectory` | MoveIt → 可选原生调试窗口 | MoveIt 原生预览；默认窗口使用自研轨迹话题 |

## 流程一：提交目标和获得规划结果

```text
命令行参数 → plan_goal（临时 /arm_plan_goal） ─┐
                                             ├─ PlanMotion Action → /arm_planning
其他节点 → /arm_goal/target → 常驻 /arm_goal ─┘
```

1. CLI 将 `--joints` 或 `--pose` 转为 Goal；Topic 模式将 MotionTarget 转为相同的 Goal。两种模式共用 C++ Action 提交流程。
2. `/arm_planning` 检查格式和忙状态。只允许一个任务，忙时拒绝，不排队。
3. 它从 `/joint_states` 的本地监视器读取起点，并通过 `/get_planning_scene` 获取场景，在本进程中检查起点碰撞。
4. JOINT 目标直接构造目标关节角；POSE 目标在本进程调用 KDL，多种子求 IK、剔除碰撞解、去重并选择较近候选。目标约束 `link_6` 的完整位姿，目前没有独立标定 TCP。
5. 调用 C++ `MoveGroupInterface::plan()`。这个本地函数内部通过 `/move_action` Action 向 `/move_group` 请求规划；OMPL/RRTConnect 在 MoveIt 进程内搜索，时间参数化也由 MoveIt 流水线处理。
6. 返回轨迹后，重新获取场景，按关节步长插值并在本进程复查碰撞。
7. 复查通过，将轨迹发布到 `/arm_planning/display_planned_path` 给 RViz 播放。
8. `execute=false` 时直接完成任务，返回 PlanMotion Result。CLI 打印/按需导出 JSON；Topic 客户端发布带 request_id 的 MotionResult。

外层 Action 的 Feedback 在过程中返回 checking_start、solving_ik、planning、validating，执行时还会有 executing；瞬时阶段不保证都能被观察到。Topic 客户端将收到的反馈转发到 `/arm_goal/feedback`，并添加 request_id。本地 accepted 阶段只表示 Topic 客户端接收了请求，不表示规划服务一定接受或成功。

## 流程二：请求模拟执行

当 execute=true，完成规划和复查后继续：

```text
/arm_planning
  │ /execute_trajectory Action
  ▼
/move_group
  │ /arm_controller/follow_joint_trajectory Action
  ▼
arm_controller
  │ ros2_control 进程内命令接口
  ▼
GenericSystem 模拟硬件
```

服务端执行前还会检查当前关节状态没有明显偏离规划起点。控制器按轨迹时间产生关节位置指令。执行结果沿控制器→MoveIt→自研规划节点→客户端返回；请求执行时，外层最终结果要等执行结束后返回。

GenericSystem 用于控制通信和状态演示，不模拟真实重力、惯性和接触动力学。

## 流程三：持续的状态和显示反馈

```text
GenericSystem 的关节状态接口
    ↓ ros2_control 进程内读取
joint_state_broadcaster
    ↓ /joint_states
    ├──→ /move_group：更新当前状态、监视执行
    ├──→ /arm_planning 的状态监视器：供规划读取起点
    └──→ /robot_state_publisher：根据 URDF 计算正运动学
                   ↓ /tf
                  RViz：更新当前机器人

/world_to_base → /tf_static → TF 消费者：固定 world→base_link
/arm_planning → 轨迹显示 Topic → RViz：播放橙色规划预览
/move_group → 场景 Topic → RViz：显示规划场景
```

这条状态流没有新目标时也持续存在。橙色预览来自动画轨迹；当前机器人来自 TF/关节反馈。只规划不会改变当前关节状态。默认 RViz 没有原生规划按钮，不发送业务目标；原生调试窗口的按钮直接访问 MoveIt，会绕过自研业务检查，应与默认业务入口区分。

## 取消与退出

- Topic 发布者将 request_id 发到 `/arm_goal/cancel`；常驻客户端把匹配的请求转换为外层 Action 取消。未知编号只反馈 cancel_ignored。
- CLI Ctrl+C、Topic 客户端 Ctrl+C 和客户端结果超时，也会尝试取消已接收任务。规划服务在阶段检查中退出，并持续调用 MoveGroupInterface::stop() 中断下游执行，直到当前任务结束。
- 根目录 run.sh 的 Ctrl+C 负责进程级统一关闭；这与单独取消一个规划任务不同。
- 每次 Topic 新任务使用不同的非空 request_id。目标和结果使用 Reliable + Volatile，先启动订阅者，再发布一次目标；不重放启动前的历史命令。

## 查看实际运行信息

```bash
source .cache/ros2/install/setup.bash
ros2 node list
ros2 node info /arm_goal
ros2 node info /arm_planning
ros2 action list -t
ros2 topic list -t
ros2 param get /arm_goal target_topic
ros2 topic echo /joint_states --once
```

包名 act_interfaces 只改变类型前缀，地址仍由 YAML 和标准配置决定；两个节点要通信，必须同时匹配接口名称、类型和相应 QoS。
