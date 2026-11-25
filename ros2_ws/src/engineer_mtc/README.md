# engineer_mtc

同一个 ROS 包提供外部取矿和持矿存车。取矿由两个 MTC Task 串接，存矿独立读取当前关节状态。

- `src/pick/task.cpp` 分别组装阶段 1，以及阶段 2–3。第二个 Task 以第一个 Task 的规划末态为起点。
- `src/place/task.cpp` 组装阶段 4–6。各阶段实现在同目录的 `stage_N.cpp`。
- `src/node/` 保存节点业务入口、分阶段执行和目标预览；程序入口为 `src/main.cpp`。
- `src/shared/` 保存运动阶段构造、矿场景、抓取几何换算和连续视觉观测缓存。
- `include/engineer_mtc/` 按 `node/`、`pick/`、`place/`、`shared/` 与源码对应。各模块的 `task.hpp` 声明任务工厂，`stages.hpp` 声明业务阶段，共用参数位于 `shared/task_settings.hpp`。

## 阶段

| 阶段 | 动作 |
|---|---|
| 1 | 开爪，规划到预抓取位 |
| 2 | 沿 TCP 方向直线接近，闭爪，将矿附着到末端 |
| 3 | 保持夹持，沿矿坐标系的拉取轴拔离 |
| 4 | 从实测关节位置持矿搬运到预存矿位 |
| 5 | 直线入槽，松爪，解除附着，直线退爪 |
| 6 | 回到 `home`，闭合空夹爪 |

## 服务

1. 调用 `/engineer_mtc/plan` 使用视觉缓存，或调用 `/engineer_mtc/plan_manual` 使用手动缓存。一次调用会规划两个取矿 Task，并在 `/engineer_mtc/display_planned_path` 发布阶段 1–3 的合并预览。
2. 调用 `/engineer_mtc/execute` 开始取矿。默认 `step`，每次收到 `WAITING_NEXT` 后调用 `/engineer_mtc/next_stage`。模式 `auto` 会自动执行当前任务内的后续阶段。
3. 阶段 3 返回 `SUCCEEDED` 并报告矿已附着后，调用 `/engineer_mtc/plan_place`。它查询场景中的持矿变换，并用 MoveIt 当前关节状态规划阶段 4–6。随后再次调用 `execute`，分步模式继续使用 `next_stage`。
4. `/engineer_mtc/cancel` 取消当前任务。存车规划失败时可以保持持矿状态重试。接触阶段执行中断后，请检查实物和场景再复位节点。执行器会在确认旧轨迹停止后恢复临时碰撞许可，并保留已经发生的附着或释放。

接口的服务类型都是 `std_srvs/srv/Trigger`。例如在模拟终端调用存车规划：

```bash
ros2 service call /engineer_mtc/plan_place std_srvs/srv/Trigger '{}'
```

`/engineer_mtc/pick_status` 保留任务号、阶段、夹爪与附着状态。阶段 0 表示尚未开始动作。状态值 0–7 依次对应 `IDLE`、`PLANNING`、`READY`、`EXECUTING`、`WAITING_NEXT`、`SUCCEEDED`、`FAILED`、`CANCELED`。阶段 3 的 `SUCCEEDED` 表示取矿结束，阶段 6 的 `SUCCEEDED` 表示存放和回位完成。

`/engineer_mtc/display_planned_path` 预览当前任务。`object_pose_base`、`grasp_tcp`、`pregrasp_pose` 分别显示取矿所用的基座下矿模型原点、抓取 TCP、预抓取 `link_6`。这些话题位于 `/engineer_mtc` 下。`/gripper_close` 中 True 表示闭合，False 表示打开。

## 碰撞与槽位配置

- 外部取矿三阶段按 26 的视觉取矿规则临时放行机器人连杆之间及与场景物体之间的碰撞。存车期间放行机械臂与车体底座的碰撞，搬运和入槽时也放行所持矿块与机器人连杆的碰撞。
- `storage_xyz` 和 `storage_rpy` 定义槽内矿的目标位姿。`storage_insertion_axis` 定义 `base_link` 下独立的入槽方向，默认模拟使用 −Z。`storage_contact_objects` 可指定已建模的槽内接触面，让手里的矿块和末端与这些接触面接触。
- 解附着后允许放下的矿接触底座，并在退爪期间保留夹持接触和已配置的末端碰槽许可。槽内矿以 `<object_id>_stored` 留在场景中，保留对底座的接触许可，并参与与其他连杆的碰撞检查。回位结束后恢复机械臂与底座的碰撞检查。此 ID 存在时存车请求会报槽位已占用。确认在模拟中取走后，可从规划场景移除这个对象以再次存放。

## 静止目标多帧取样

`config/engineer_mtc.yaml` 的 `target_average_samples` 默认 5，表示使用最近 5 次连续有效视觉位姿；设为 1 恢复单帧，修改后重启 MTC。
MTC 不直接平均点云：每帧位姿先用其采集时间戳变换到 `base_link`，再对位置取算术平均，朝向使用四元数特征向量平均，避免直接平均欧拉角或相反符号的四元数。
未规划时的场景预览也使用相同均值；规划成功后使用固定目标执行，后续新观测不改变既有任务。手动目标仍使用单帧。

最新观测必须满足 `max_target_age`（默认 1.5 秒）；窗口历史样本分别受 `target_average_max_age`（默认 5 秒）限制，适配视觉输出较慢的静止取样。均值保留最旧样本时间戳，不以当前时间刷新旧观测。
漏检、无效位姿、坐标系或目标数量变化、时间倒退、观测间隔超时、相邻朝向变化超过 90°时清空窗口。重复时间戳不重复计数。
样本不足时 `plan` 返回具体数量并拒绝规划，不使用不足窗口或历史空目标凑数。
例如视觉约 5 Hz，5 帧覆盖约 0.8 秒；增大帧数时需同时考虑历史窗口 5 秒和最新观测 1.5 秒这两个独立限制。

此功能适合相机和物体相对基座静止的场景；它减少小幅随机抖动，不能证明大小端识别正确。
多个目标的身份仍沿用既有 `target_index`，相同数量下的目标重排序不属于身份跟踪保证。
