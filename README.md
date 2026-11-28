
## 远程 MTC 预览

Jetson 运行 `./run.sh mode:=mtc use_rviz:=false`，Ubuntu 22.04 电脑运行 `./run_rviz.sh`。

## 启动配置

编辑 `config/1.json` 后运行 `./run.sh`。

```json
{
  "debug": true,
  "mode": "mtc",
  "use_rviz": true,
  "start_camera": true,
  "start_detector": true,
  "start_icp": true,
  "start_bridge": true
}
```

`debug: true` 时，`mode` 选择 `mtc` 或 `vision`。`use_rviz` 控制机械臂 RViz，四个 `start_*` 开关用于视觉模式。`start_icp` 在 `start_detector` 开启时生效。省略 `debug` 时默认开启。

`debug: false` 时，同时启动硬件控制、MoveIt、MTC、相机、检测、模型点云、ICP、Foxglove Bridge 和录像节点，RViz 仍由 `use_rviz` 控制。此时使用各包默认 YAML 和节点默认解释器，其余 JSON 启动字段及其命令行覆盖跳过。录像是否保存视频由相机 YAML 的 `video` 控制，硬件选择由串口 YAML 的 `use_mock_hardware` 控制，运动由 MTC 执行服务触发。

可以添加 `hardware_config`、`controller_config`、`sim_config`、`mtc_config`、`camera_config`、`vision_config`、`foxglove_config` 和 `yolo_python` 字符串字段，指定配置文件或 Python 路径。JSON 中的相对路径以 JSON 所在目录为基准，字段设为 `null` 或省略时沿用启动默认值。各包 YAML 保存硬件选择、算法和规划参数。

临时覆盖可用 `./run.sh mode:=vision`，选择另一份配置可用 `./run.sh --config config/2.json`。命令行同名字段优先。

### 集中调参

`config/1.json` 的 `parameters` 按节点保存常调参数，支持 `engineer_mtc`、`serial_driver`、`video_recorder`、`icp_registration` 和 `yolo_detector`。当前集中项如下

```json
"parameters": {
  "engineer_mtc": {
    "execution_mode": "auto"
  },
  "serial_driver": {
    "mock_mode": 2,
    "use_mock_hardware": false,
    "serial_port": "/dev/ttyACM0"
  },
  "video_recorder": {
    "video": false
  },
  "icp_registration": {
    "use_hexagon_prior": true
  },
  "yolo_detector": {
    "use_manual_roi": false,
    "auto_lock_detection": true,
    "conf_thres": 0.7,
    "iou_thres": 0.45,
    "device": "gpu",
    "imgsz": 640
  }
}
```

`debug: true` 时，启动脚本先读取包内 YAML，再合并显式配置文件，最后应用 `parameters`。合并结果保存在 `.cache/startup/`，并通过现有 launch 参数传给节点。设置为 `null` 或移除某个参数即可沿用基础配置。字段名称和类型按基础 YAML 校验，距离单位沿用原参数定义。

`mock_mode` 仅在模拟硬件下生效，1 为手动，2 为自动取矿，3 为兑矿预留。`execution_mode` 可选 `auto` 或 `step`。

修改后重新运行 `./run.sh` 生效。`debug: false` 时使用包内比赛配置，跳过 `parameters` 覆盖。

## RViz 任务控制

构建 `sim_controller` 后，本机和远程 MTC RViz 都会加载 **MTC 控制**面板，提供视觉取矿规划、手动目标规划、持矿存车规划、执行、下一阶段和取消按钮。面板调用现有 MTC 服务，并显示 `/engineer_mtc/pick_status` 的任务阶段和结果。

本机和远程使用同一份 `sim_controller/config/mtc.rviz`，包含完整的 MTC 阶段与解浏览、轨迹动画、规划场景、当前机器人、取矿与存车 TCP 标记，以及当前矿和槽内矿显示。

需要逐阶段调试时，将 JSON 中的 `parameters.engineer_mtc.execution_mode` 设为 `step`，规划后点击执行，收到等待下一阶段状态后点击下一阶段。`auto` 模式连续执行当前任务。手动目标规划使用节点已有的手动位姿缓存。

## 辅助工具

`tools/` 根目录保留直接运行的入口

- `build_energy_model.py` 根据矿 STL 生成 URDF 和碰撞几何报告
- `preview_robot.sh` 一键查看完整 URDF、所有坐标系和关节角度滑块
- `preview.launch.py` 独立预览 URDF，支持关节滑块和 TF 显示
- `preview_camera.sh` 预览机械臂上的 D435 示意外壳和坐标系

`tools/rviz/` 保存模型预览用的 RViz 配置，`tools/internal/` 保存启动配置解析和 ROS 环境辅助脚本。在终端切换到工程通信域可执行 `source tools/internal/ros_env.sh`。

运行 `./tools/preview_robot.sh` 打开当前 `assets/engineer/arm.urdf`。RViz 显示完整模型及坐标系名称，X/Y/Z 轴分别为红/绿/蓝；展开左侧 TF 的 Frames 可查看各坐标系的位置、方向和父子关系。另一个 Joint State Publisher 窗口可拖动滑块调整关节角度，旋转关节数值单位为弧度，模型和坐标系会同步更新。此工具仅发布预览关节状态和 TF，使用独立本机 ROS 域。
