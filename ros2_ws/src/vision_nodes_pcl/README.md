# vision_nodes_pcl

在 27 内复现 `engineer_2026/jetson` 的 `target_detection` C++／PCL 流程，提供与
`vision_nodes` 同阶段的相机、YOLO、ROI、配准、图像和点云显示能力。没有连接机械臂执行。

## 启动

在 Engineer_27 根目录的系统 ROS 终端执行：

```bash
./build.sh --packages-select vision_nodes vision_nodes_pcl
./run.sh mode:=vision_pcl use_rviz:=true
```

原来的 `./run.sh mode:=vision` 仍使用 Python／Open3D。两种模式共用视觉运行锁；先在旧终端
Ctrl+C 再切换，不能在同一域同时发布相同话题和争用 D435。新包配置独立放在
`vision_nodes_pcl/config/`，修改这里的 `vision_params.yaml` 不会改变 Open3D 版。

也可在任意目录加载本工作区后启动：

```bash
source /实际路径/Engineer_27/ros2_ws/install/setup.bash
ros2 launch vision_nodes_pcl vision_pipeline.launch.py
# 已运行相机或回放时按需设置：start_camera:=false start_bridge:=false
# 单独附加显示，不启动算法或相机：
ros2 launch vision_nodes_pcl vision_rviz.launch.py
```

启动参数与 Python 包相同：`start_camera`、`start_detector`、`start_icp`、`start_bridge`、
`camera_config`、`vision_config`、`foxglove_config`、`yolo_python`；PCL 不使用 `icp_python`。
`start_detector:=false` 时只转发原图，也不启动模型和 ICP；回放已记录的检测时可单独运行
`model_cloud_publisher` 和 `icp_registration` 可执行文件并显式传入参数文件。

YOLO 复用 `vision_nodes/yolo_detector`，使用 Conda engineer 的 Python 和当前 27 的权重及检测参数；
没有复制另一份推理代码。PCL 配准、模型发布和 main_visualizer 都是 C++，不需要 Open3D。
共享 detector 也支持固定手动框：本包 `config/vision_params.yaml` 中的 `use_manual_roi=true`
会弹出鼠标选框窗口，跳过 YOLO，持续使用上次完成的矩形直到再次框选。拖动期间继续用旧框，
关闭窗口不停止 ROI 输出。运行中可通过 `ros2 param set /yolo_detector use_manual_roi true/false`
切换；`ros2 service call /yolo_detector/select_roi std_srvs/srv/Trigger '{}'` 可重新显示窗口。
需保持 `start_detector=true`；详细交互、时间戳和重启语义见
[手动 ROI 说明](../vision_nodes/README.md#鼠标框选固定-roi两条配准链路共用)。
模型和权重源文件统一保存在根目录 `assets/`，构建后安装到本包 share/assets，默认相对资产路径
以安装 share 为基准；自定义权重或模型也可使用绝对路径。

## 文件职责

```text
include/vision_nodes_pcl/       节点声明、计算接口、约束配置
src/registration.cpp           模型读取、投影、SOR/体素、原版 PCL ICP 与计分
src/pose_constraints.cpp       26 六矿布局和 Z 轴约束，保留原公式及调用顺序
src/icp_registration_node.cpp  ROS 输入、ROI、坐标转换、上一结果缓存、调试输出和失效清空
src/model_cloud_publisher_node.cpp  CAD 点云定时发布
src/main_visualizer_node.cpp   精确同帧图像/检测配对，按需画框
src/*_main.cpp                 初始化、spin、异常退出
config/                       各节点 YAML、相机、Bridge 和 RViz 布局
launch/                       视觉流程和独立 RViz
test/                         已知变换、约束、真实 ROS 收发与失效回验
```

## 与 26 保持一致的流程及数值

1. `assets/energy.ply` 原顶点 → 乘 `scale=0.001` → RandomSample 限制最多 5000 点
   → 5 mm 模型体素 → 100 Hz 发布；ICP 接收源模型后再次按 6 mm 体素过滤。
   这是顶点限点，不是 Python 版 STL 的 2000 点泊松盘表面采样。
2. 当前深度云 + **最新检测数组** + 缓存彩色 K，不增加 Python 版的近似同步器。
3. 原相机坐标下欧氏距离 `max_range=1.5 m` → 针孔投影 → YOLO 框内 ROI；
   `bbox_shrink_ratio=0`，非零时每一边各收缩该比例，最大 0.45。
4. 先 SOR，`sor_mean_k=20`、`sor_std_mul=0.8`，再 6 mm 体素；原始 ROI 和过滤后都要求至少 80 点。
5. 将目标云变换到 `target_frame`，以上一检测序号的成功 ICP 变换初始化；没有缓存时用质心差初始化平移，初始旋转单位阵。
6. PCL 点到点 ICP：`max_icp_iter=120`、`max_corr_dist=1.0 m`；不额外改变 PCL 默认停止条件。
7. 要求 `hasConverged()`；计算 `score=getFitnessScore()`（MSE，m²），
   `metric=1/(1+score)`；要求 `metric>=0.92`、`score<=0.01`。
   `max_fitness_score=0` 仅禁用 MSE 上限；不是 Python 的 `max_inlier_rmse`。
   `0.01 m²` 的平方根是 100 mm，不能按 10 mm 理解，也不是 Open3D fitness 覆盖率。
8. 保存完整 ICP 变换用于下一帧；输出朝向先只保留物体 Z 轴方向，再依次执行：
   六矿状态更新 → 槽位吸附 → Z 轴向外 → Z 轴角度约束。
   六矿先验所有数值与开关都沿用 26 YAML，不属于通用的配准评分。

上一结果是按检测数组下标缓存，不是稳定目标 ID，也不会在 YOLO 消失时独立跟踪点云。
六矿先验是旧资源布局假设，当前打开是为了复现；不代表已经验证 27 站体、手持矿或倾斜相机满足该假设。

## 为接入 27 做的明确适配

- 26 默认 `target_frame=base_link`；独立 D435 没有这个 TF，因此这里默认 `camera_link`。
  将来配置真实外参后可改回 `base_link`，不发布虚构的 base_link 变换。
- 原模型标为独立 `energy_model`，没有固定 TF。几何数值直接作为 CAD 输入，
  不把模型当成已放置在相机/机器人位置的实体；对应 26 源/目标标签相同而未执行源 TF 的计算路径。
- **保留 26 投影方式**：直接用深度 XYZ 和彩色 K，不做投影前的深度到彩色外参或畸变修正。
  这与 27 Python 版不同；框内点可能存在投影偏差，复现成功不代表这项假设正确。
- 增加过滤前云、失败时仍可查看的过滤后云、原始 ICP 和最终约束模型输出；多 ROI 聚合显示。
  几何计算保持 PointXYZ，RGB 只供显示，过滤后颜色取原 ROI 最近邻，不参与计分。
- 空检测/拒绝结果发布空消息；输入超过 `max_data_age=1.5 s` 或断流会清空输出与历史缓存。
  这是与 27 对齐的失效语义，不是 26 原版的“只在成功时发布”。
- 模型发布 QoS 增加 transient_local，启动时资产缺失直接报错；不悄悄发布空模型。
- 输入 QoS 默认改为与 27 相同的 reliable，避免本机大点云 best_effort 分片丢失；
  `input_reliability=best_effort` 可恢复 26 的输入可靠性，不改变配准参数或计算。

`camera_link` 下 XY 平面不等于已标定的资源站平面。更换目标坐标系会影响六矿几何约束的含义。
PLY 与 STL 的原点也不同，不能把两套位姿直接相减或直接作为夹爪 TCP 目标。

## 订阅、发布与观察

| 节点 | 订阅 | 发布 |
| --- | --- | --- |
| D435 官方驱动 | — | `/camera/camera/...` 彩色图、深度图、CameraInfo、场景点云、相机 TF |
| 共享 yolo_detector | 彩色图 | `/vision/yolo/detections`，Detection2DArray，保留采集时间戳 |
| model_cloud_publisher | — | `/vision/model_cloud`，PointCloud2，CAD 模型 |
| icp_registration | 场景点云、彩色 CameraInfo、最新检测、CAD 点云及 TF | 下表各点云、PoseArray；有效最终姿态的 `object_<检测序号>` TF |
| main_visualizer | 同采集时间戳的彩色图和检测 | `/vision/visualization/image`，Image，独立画框 |
| Foxglove / RViz | 上述可视化数据 | 显示，不参与配准 |

| 话题 | 含义 |
| --- | --- |
| `/vision/icp/roi_cloud` | 距离筛选、框选后，SOR/体素前，RGB |
| `/vision/icp/target_cloud` | SOR/体素后的观测，验收失败仍输出，RGB |
| `/vision/icp/raw_aligned_cloud` | 通过 ICP 评分的原始完整变换模型，尚未施加 Z 轴/六矿约束 |
| `/vision/icp/raw_object_poses` | 对应原始完整 ICP 变换，PoseArray |
| `/vision/icp/aligned_cloud` | 按最终发布位姿放置的模型，含 Z 轴/六矿约束 |
| `/vision/icp/object_poses` | 26 流程最终位姿，PoseArray |

除 CAD 云外，上述 ICP 输出都使用 `target_frame`（为空则使用输入点云坐标系）及输入云时间戳，单位 m。
位姿原点是 PLY 的 CAD 原点。原始与最终数组按同一成功候选顺序输出，不承诺跨帧身份。
无结果时云和 PoseArray 都为空；TF 历史不能撤销，因此消费者必须以当前 PoseArray 为有效性依据，
不能因为 TF 缓存里还能查到旧 object 就继续执行动作。

RViz 的相机视角与 Python 包相同：绿色观测、红色约束后模型；需要检查 ICP 本身时关闭
`Final Model after constraints (red)`，打开 `Raw ICP before constraints (orange)`。
CAD 模型没有相机 TF，应单独将 Fixed Frame 设为 `energy_model` 后查看 `/vision/model_cloud`。
Foxglove 连接 `ws://127.0.0.1:8765`，Image 选 `/vision/visualization/image`，3D 选上述话题。

## 验证

```bash
cd /实际路径/Engineer_27/ros2_ws
source install/setup.bash
colcon test --packages-select vision_nodes_pcl --event-handlers console_direct+
colcon test-result --test-result-base build/vision_nodes_pcl --verbose
```

单元测试覆盖已知刚体变换、无对应点拒绝、MSE 上限禁用、PLY 尺度/原点、Z 轴与六矿约束。
ROS 测试使用独立域 196，启动本包三个 C++ 节点，验证完整消息链、同帧画框、原始/最终朝向差异、
空检测和断流清空，不启动相机、YOLO、Bridge 或机械臂。合成数据通过不代表实物定位精度。

2026-11-14 本机验证：构建和测试通过。将 4 帧已记录的完整 XYZRGB/检测框同时送入本包和
26 原始计算实现（仅适配 reliable 传输、资产路径和输出坐标系），过滤后点数分别为
538、839、565、526；观测点坐标、最终位置和四元数逐项一致。完整离线启动还验证了实际 YOLO
推理 → PCL → C++ 画框 → Foxglove WebSocket 图像输出。记录位于根目录
`.cache/pcl_validation/validation.md`，没有启动相机或机械臂；本次未新增 RViz GUI 验证。

参考源：`engineer_2026/ros2_ws/src/target_detection/`（MIT，见原 package.xml）。本包保留对应
配准/先验行为，重组为 27 的 include/src/config/launch/test 风格；相机与显示配置来自 27。
