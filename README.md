# Engineer 27：离线路径规划

目前使用现有机械臂模型完成 MoveIt 配置、碰撞检查、关节目标与末端位姿目标规划，以及模拟执行。没有接入视觉、串口或真实电机。

## 目录

```text
assets/                                      # 外观、碰撞几何与原始资料
scripts/                                     # 离线网格处理工具（Conda）
ros2_ws/src/
  engineer27_description/                    # 复用 assets，提供模型资源
  engineer27_moveit_config/                  # SRDF、IK、OMPL、模拟控制、RViz
  arm_planning/                             # 规划 Action、C++ 服务节点与命令行
.cache/planning_validation/                  # 本机验证日志与结果，不是运行依赖
```

## 构建和启动

使用 ROS 2 Humble 的系统环境。这部分不需要 Conda；若已激活 Conda，先退出，避免其 Python 或动态库影响 ROS。

```bash
cd /home/starbeloved/code/Robomaster/Engineer_27/ros2_ws
source /opt/ros/humble/setup.bash
colcon build --symlink-install --packages-up-to engineer27_moveit_config
source install/setup.bash
ros2 launch engineer27_moveit_config demo.launch.py
```

启动前关闭旧的 `assets/preview.launch.py`，避免两个程序同时发布同一套关节状态。启动后等待 MoveIt 和模型加载完成。在 RViz 的 MotionPlanning 面板中选 `arm_control`，可以用目标交互标记或关节值设置目标，点击 **Plan** 规划，点击 **Execute** 让模拟关节沿轨迹运动。界面直接使用 MoveIt；下面的命令行另有候选 IK 选择和轨迹复查。

无需显示界面时：

```bash
ros2 launch engineer27_moveit_config demo.launch.py use_rviz:=false
```

之后需要附加 RViz 时，在同一 ROS 域运行 `ros2 launch engineer27_moveit_config rviz.launch.py`，不会重复启动控制器和规划服务。

## 提交规划目标

另开终端，同样 `source /opt/ros/humble/setup.bash` 和工作区的 `install/setup.bash`。

关节目标顺序固定为 `link_1_joint` 至 `link_6_joint`，单位为弧度。默认只规划、在 RViz 显示轨迹，不改变当前关节状态。

```bash
ros2 run arm_planning plan_goal --joints 0.25 0 0 0 0 0
ros2 run arm_planning plan_goal --joints 0.25 0 0 0 0 0 --execute
ros2 run arm_planning plan_goal --joints 0 0 0 0 0 0 --execute --output /tmp/home_plan.json
```

`--execute` 只通过本项目启动的 `mock_components/GenericSystem` 模拟控制器执行。JSON 输出包含关节位置、速度、加速度和各轨迹点的时间。

末端目标使用 `--pose X Y Z QX QY QZ QW --frame base_link`；位置单位为米，姿态是单位四元数；也支持 `world` 坐标系。目前目标对应 **link_6 的原点**，尚未建立实测 TCP。可以先用 `/compute_fk` 从已知有效关节姿态得到一个可达目标，再尝试位姿规划。

下面是已从当前模型 FK 得到并验证过的目标示例（只规划）：

```bash
ros2 run arm_planning plan_goal --pose 0.0534446 0.00930325 0.19012849 -0.00996671 0.09933467 0.09933467 0.99003329
```

常用参数：`--planning-time 5`、`--velocity-scale 0.2`、`--acceleration-scale 0.2`、`--timeout 120`。后两项是相对于配置中速度、加速度上限的比例，范围 `(0, 1]`。Action 中传 0 时使用默认值。规划时限只限制 OMPL 求解，整个请求还包括 IK、场景读取和轨迹复查。

## 规划模块的行为

`/arm_planning/plan_motion` 使用自定义 `arm_planning/action/PlanMotion`：

1. 读取当前关节反馈与 MoveIt 场景，检查起点。
2. 关节目标将连续关节角映射到接近当前姿态的一圈；位姿目标用 KDL 从 12 个种子寻找无碰撞 IK，在已找到的不同候选中选关节变化平方和最小的一组。
3. 检查目标，调用 OMPL RRTConnect 搜索路径，再做时间参数化。
4. 再次读取场景，对轨迹插值，默认每轴角度步长不超过 0.01 rad，检查限位、自碰撞和场景障碍物。
5. 发布轨迹；若请求执行，还会检查当前状态是否偏离规划起点，再调用模拟轨迹控制器。

同一时刻接受一个请求，支持取消。不可达目标、干涉目标和无效输入会返回失败，不会执行。外部障碍物需通过 MoveIt 场景接口（例如 `/apply_planning_scene`）加入；没有视觉时也能手动添加。

## 当前配置的边界

- SRDF 只排除 6 对直接相邻连杆的碰撞，其余连杆之间继续检测。相邻安装部位的豁免需要后续结合实物复核。
- `[0, 0, 0.1, 0, 0, 0]` 是当前模型的已知干涉目标，涉及 `link_1` 与 `link_4/link_5`。没有通过扩大豁免表让它通过。
- 保留原 URDF 的 6 个 continuous 关节，没有根据仿真猜测真实角度限位。速度 1 rad/s、加速度 1 rad/s² 是离线规划配置，默认再乘 0.2；尚未按电机性能核定。
- IK 只在有限种子预算中选择附近解，不保证找全所有解，也未加入奇异性评分。首个候选无法规划时，当前版本返回失败，不自动遍历其余候选。
- 路径检查是离散插值检查，不是连续碰撞证明；场景在校验后发生变化仍需重新规划。当前定位为静态场景的离线验证。
- 真机接入前还需要实测限位、零位与 TCP 标定、碰撞模型复核及控制接口。

## 集成验证

使用独立 ROS 域启动无界面模拟，再运行检查。测试会执行模拟运动、添加并移除测试障碍物，以及取消轨迹，勿对真实控制器运行。

```bash
# 终端一（已 source 工作区）
ROS_DOMAIN_ID=187 ros2 launch engineer27_moveit_config demo.launch.py use_rviz:=false

# 终端二（已 source 工作区，从 Engineer_27 目录运行）
ROS_DOMAIN_ID=187 /usr/bin/python3 ros2_ws/src/arm_planning/test/offline_integration.py
```

检查内容包括已知干涉与碰撞矩阵、连续关节就近取角、关节路径、位姿 IK 与 FK 回验、不可达目标、世界障碍物、输入校验、模拟执行、并发拒绝和取消，以及起终点有效但直接关节插值发生干涉时的绕行规划。结果写入 `.cache/planning_validation/integration_results.json`。
