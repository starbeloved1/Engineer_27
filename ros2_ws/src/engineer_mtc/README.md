# engineer_mtc

基于 MoveIt Task Constructor（MTC）的能量单元取矿任务，完成抓取、拉取、车载存矿和回位

流程：视觉位姿 → 转换到基座坐标系 → 计算抓取目标 → 加入矿碰撞模型 → 规划完整 Task → 分阶段执行

## 取矿 Task

1. **stage 1：预抓取。**按 26 放宽碰撞，打开夹爪，规划到预抓取状态。
2. **stage 2：夹取。**沿 TCP 接近方向直线移动，闭爪，将矿附着到末端。
3. **stage 3：拉取。**保持夹持，沿矿坐标系的拉取轴外移，默认沿 PLY 的 +Z 拉取 0.10 m
4. **stage 4：搬运。**将矿搬运到车载预存矿位置。
5. **stage 5：存矿。**沿存矿坐标系的 -Z 放入，打开夹爪、解除附着，再反向退出，保持宽松碰撞策略。
6. **stage 6：回位。**清理场景物体，机械臂回到 `home`，闭合空夹爪。

执行由 `src/core/pick_executor.cpp` 管理：

- **auto**：每阶段执行成功并确认反馈后，自动进入下一阶段。
- **step**：每阶段完成后暂停，调用 `next_stage` 才继续。

## 话题含义说明

`/engineer_mtc/pick_stage`表示业务阶段，**0** 表示尚未进入动作，**1～6** 对应上述阶段

`/engineer_mtc/pick_status`包含任务编号、阶段、运行状态、夹爪与附着状态、结果说明。

- **0**：IDLE，空闲。
- **1**：PLANNING，规划中。
- **2**：READY，完整规划就绪，等待执行。
- **3**：EXECUTING，当前阶段执行中。
- **4**：WAITING_NEXT，分步模式下等待继续。
- **5**：SUCCEEDED，完整任务完成。
- **6**：FAILED，规划或执行失败。
- **7**：CANCELED，已取消。

`/gripper_close`用于指示夹爪开合，**True 为关，False 为开**

`object_attached`表示规划场景中的附着关系。

目标和轨迹预览话题：

- `/engineer_mtc/object_pose_base`：本次规划使用的基座下 PLY 原点位姿。
- `/engineer_mtc/grasp_tcp`：基座下抓取 TCP 目标。
- `/engineer_mtc/pregrasp_pose`：基座下预抓取 `link_6` 目标。
- `/engineer_mtc/display_planned_path`：完整规划轨迹预览。


## 服务与执行接口

- `/engineer_mtc/plan`：使用视觉缓存规划完整任务
- `/engineer_mtc/plan_manual`：使用手动缓存规划完整任务
- `/engineer_mtc/execute`：开始执行最近成功的完整规划
- `/engineer_mtc/next_stage`：分步模式下继续下一阶段
- `/engineer_mtc/cancel`：请求取消任务


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
