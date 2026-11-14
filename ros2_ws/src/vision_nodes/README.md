# vision_nodes：取矿视觉节点

本包集中管理视觉节点，参考 `engineer_2026/jetson` 的 Python 包组织方式，以及 `Hero_27/heros` 的参数归属、按需发布和入口职责划分。`assets/best.pt` 和 `assets/energy.stl` 均复制自 26，保留参考仓库原文件；运行不依赖参考仓库路径或安装环境。

目前包含 D435 采集、YOLO 二维检测、框内点云提取、局部 ICP 和独立画框。已用合成数据检查坐标转换、配准与失效清空；实物矿识别率、配准精度和可靠初始朝向尚待验证。视觉输出尚未接机械臂。

## 数据流与职责

```text
D435 → 官方驱动 /camera/camera
  ├─ 彩色原图 → /yolo_detector → /vision/yolo/detections
  │      └────────────────┬────────────┘
  │                  /main_visualizer → /vision/visualization/image
  ├─ 彩色 CameraInfo ───────────────┐
  ├─ 彩色场景点云 ─────────────────┤
  └─ 相机内部 TF ──────────────────┤
                         /icp_registration ← assets/energy.stl
                            ├─ /vision/model_cloud（原模型）
                            ├─ /vision/icp/roi_cloud（框内未下采样观测）
                            ├─ /vision/icp/target_cloud（框内观测）
                            ├─ /vision/icp/aligned_cloud（通过阈值的模型叠合）
                            └─ /vision/icp/object_poses（通过阈值的位姿）

相机、视觉和 TF 话题 → /foxglove_bridge → Foxglove Image / 3D
```

- `yolo_detector`：订阅彩色原图，只发布框、类别和置信度，不绘图。
- `main_visualizer`：原图与检测按完全相同的采集时间戳配对，检查坐标系，在副本上画框。没有显示端订阅时不生成调试图。关闭检测时转发原图。
- `icp_registration`：模型加载、单位转换和采样只在启动时做一次；每对采集时间接近的点云／检测执行框内筛选、体素和离群点过滤、局部点到点 ICP。处理不依赖 Foxglove 是否打开。
- `pointcloud_geometry.py`：几何算法；`pointcloud_messages.py`：PointCloud2 与 NumPy 转换。
- launch：加载所属 YAML，管理各节点，任一进程退出时停止同组进程。

图像可视化由 `main_visualizer` 绘制。场景云、目标云和模型叠合云都是标准三维数据，直接由 Foxglove 的 3D 面板显示，不渲染成二维图像。

## 构建与启动

终端使用系统 ROS 2 Humble，无需激活 Conda。launch 为 YOLO 和 ICP 选择 Conda `engineer` 的 Python 3.10，基础 ROS 节点继续使用系统 Python。当前默认 CPU 推理；Open3D 也使用 CPU。

本机依赖已安装。复现 Python 环境时：

```bash
conda activate engineer
python -m pip install torch==2.8.0 torchvision==0.23.0 --index-url https://download.pytorch.org/whl/cpu
python -m pip install -r ros2_ws/src/vision_nodes/requirements-yolo.txt
python -m pip install -r ros2_ws/src/vision_nodes/requirements-icp.txt
conda deactivate
```

先关闭 RealSense Viewer 或旧的相机流水线，在 Engineer_27 根目录运行：

```bash
./build.sh --packages-select vision_nodes
./run.sh mode:=vision
```

默认启动 D435、YOLO、ICP、main_visualizer 和 Bridge，不打开 RViz。权重／网格不存在或加载失败时直接退出，不自动下载替代文件。Ctrl+C 统一停止本入口所属进程；关闭 Foxglove 不停止采集。同一工作区和 ROS 域的重复启动由 run.sh 的锁阻止。

```bash
# 只查看 YOLO 图像与场景云：
./run.sh mode:=vision start_icp:=false
# 只查看原图与场景云，同时关闭 YOLO 和 ICP：
./run.sh mode:=vision start_detector:=false
# 复用已有相机或 Bridge：
./run.sh mode:=vision start_camera:=false
./run.sh mode:=vision start_bridge:=false
```

配置覆盖参数是 `camera_config:=FILE`、`vision_config:=FILE`、`foxglove_config:=FILE`。`yolo_python:=FILE`、`icp_python:=FILE` 分别选择解释器；默认根据 `CONDA_EXE`／`conda` 可执行路径寻找 `engineer`，找不到 Conda 时使用 launch 的 Python。

`run.sh` 将相对模型路径基准固定为 Engineer_27 根目录。单独运行 `ros2 launch vision_nodes vision_pipeline.launch.py` 时，也应在该目录，或在 YAML 中配置绝对模型路径。`start_icp` 默认 true，但只有 `start_detector=true` 时才启动；独立接入其他检测源可单独运行 ICP 节点。

## Foxglove 查看

连接类型选 Foxglove WebSocket，地址 `ws://127.0.0.1:8765`。

| 面板／用途 | 设置 |
| --- | --- |
| Image，看检测 | `/vision/visualization/image` |
| 3D，检查过滤前的框内点 | Display frame 为 `camera_link`，启用 `/vision/icp/roi_cloud`；已筛除无效点及距离范围外的点，未做体素／离群点过滤 |
| 3D，看矿的观测表面 | Display frame 为 `camera_link`，启用 `/vision/icp/target_cloud`，颜色选 RGB |
| 同一 3D，对比配准模型 | 同时启用 `/vision/icp/aligned_cloud`，选固定醒目颜色，点尺寸约 3～4 px |
| 3D，看完整场景 | `/camera/camera/depth/color/points`；检查矿时可暂时关闭，减少遮挡 |
| 单独检查原模型 | 另建 3D 面板，Display frame 设 `energy_model`，启用 `/vision/model_cloud` |

原模型 `energy_model` 不连接相机 TF，不能直接叠加到 `camera_link` 中。叠合时看 `aligned_cloud`。目标云存在而模型云为空，表示点数或拟合阈值未通过；两者都空时先检查 YOLO 框、深度、内参、TF 和采集时间差。框内云仍可能包含背景，不等于实例分割。

点太少时，先分别查看 `roi_cloud` 与 `target_cloud`。日志 `raw → voxel → sor` 对应框内距离筛选后、体素下采样后、统计离群点过滤后的点数。`raw` 已经很少时应检查原始深度、目标距离和投影；`raw` 多而 `voxel` 降得明显时可试减小 `voxel_size`（例如 0.003 m）；`sor` 又明显减少时，可临时令 `sor_neighbors=0` 对比。参数在启动时读取，改后重启。点数下降本身不表示几何信息一定丢失，应同时比较物体轮廓。

3D 面板里黑色背景可能是深度缺测或点之间的空隙，也可能存在真实 RGB 黑色点。临时将点云颜色改为固定亮色并增大点尺寸，可区分颜色与空洞。原始深度中 0 表示无有效测量，不应直接以补洞结果作为真实表面。屏幕、反光、缺乏纹理和过近目标都会影响深度，应对矿本体的 ROI 单独统计。

当前相机 USB 2 配置为 640×480、6 FPS。算法处理频率取决于推理与 ICP 耗时，不保证处理每个相机帧。Bridge 默认监听本机，话题白名单允许 `/camera/*`、`/vision/*`、`/tf` 和 `/tf_static`，显示订阅队列为 1。

## 时间、坐标系与失效

检测数组与每个 Detection2D 保留原始彩色图时间戳和坐标系；框坐标与宽高均为原图像素，theta=0，类别是字符串索引，当前 26 权重为 `0 → item`。检测本身不包含三维位姿或稳定身份。

ICP 缓存点云等待推理结果，按采集时间近似配对，默认差值小于 50 ms，每条消息最多使用一次。不会将最新框无限复用于后续云。需要更严格的动态场景同步时应进一步验证相机时间行为；当前近似同步不代表完全同时曝光。

CameraInfo 作为当前流配置的标定参数缓存。按 cloud→color 的真实 TF 将点转换到彩色光学系后，结合内参与畸变投影，再筛选 YOLO 框内点。禁止直接将深度光学坐标当成彩色光学坐标。更改相机流模式应重启流水线。

目标云、叠合模型和 PoseArray 均使用观测点云原本的 `header.frame_id` 和采集时间，单位米。位姿是模型原点与轴在该坐标系下的变换，模型原点未移到质心。模型尺寸、原点与来源见 [模型说明](../../../assets/energy_model.md)。这不是基座坐标或夹爪目标。

每个有效配对帧都发布三项结果：无检测／配准全部失败时相应输出为空。框内过滤云不依赖 ICP 是否通过。缺少内参／TF、数据过期时清空输出；超过 `max_data_age` 未获得新结果时也清空，防止显示一直停留在旧目标。源模型保持 Transient Local，其余输出 Reliable + Volatile，队列深度 1。输入默认 Reliable，可改 Best Effort。

PoseArray 只包含当前帧通过阈值的结果，按输入框遍历顺序排列；失败框会被跳过，数组下标不是输入检测下标或稳定 ID。不发布 `object_i` TF，不按上帧框序号复用位姿。后续抓取业务需要另加带身份和有效性的目标选择接口。

## 接口

| 发布节点 | 话题 | 类型与内容 |
| --- | --- | --- |
| 相机 | `/camera/camera/color/image_raw` | Image，彩色原图 |
| 相机 | `/camera/camera/color/camera_info` | CameraInfo，彩色内参与畸变 |
| 相机 | `/camera/camera/depth/image_rect_raw` | Image，16UC1 深度，本驱动输出 mm |
| 相机 | `/camera/camera/aligned_depth_to_color/image_raw` | Image，对齐到彩色像素的深度 |
| 相机 | `/camera/camera/depth/color/points` | PointCloud2，RGB 场景点云，m |
| 相机 | `/tf_static` | 相机内部坐标变换 |
| YOLO | `/vision/yolo/detections` | Detection2DArray，原图像素框、类别、置信度 |
| 可视化 | `/vision/visualization/image` | Image，BGR8 同帧画框图；关闭检测时转发原图 |
| ICP | `/vision/model_cloud` | PointCloud2，原模型坐标系下的采样点，m |
| ICP | `/vision/icp/roi_cloud` | PointCloud2，框内有效距离范围中的观测点，保留 RGB，尚未体素／SOR 过滤 |
| ICP | `/vision/icp/target_cloud` | PointCloud2，各框过滤后的观测云合并，保留 RGB，重叠框可能重复点 |
| ICP | `/vision/icp/aligned_cloud` | PointCloud2，通过阈值的模型变换后合并，无 RGB |
| ICP | `/vision/icp/object_poses` | geometry_msgs/PoseArray，当前通过阈值的模型位姿 |

## 参数

参数归属均在 `config/vision_params.yaml`，节点启动时读取，修改后重启。

- `yolo_detector`：`model_path=assets/best.pt`、`conf_thres=0.3`、`iou_thres=0.45`、`imgsz=640`、`device=cpu`。阈值用于 YOLO 检测；imgsz 为推理尺寸，输出框仍对应原图。
- `main_visualizer`：`draw_detections` 由 launch 的 `start_detector` 联动；`sync_queue_size=30`；`visualization_rate_hz=0.0` 表示跟随输入，正数限制调试图最大发布频率。
- `camera.yaml` 归 RealSense 驱动，启用深度对齐、彩色点云和内部 TF。
- `foxglove_bridge.yaml` 归 Bridge，控制地址、端口、话题白名单和订阅 QoS。

ICP 的主要调节项：

| 参数 | 默认值 | 含义 |
| --- | --- | --- |
| `model_path` / `model_scale` | `assets/energy.stl` / `0.001` | 网格路径，mm 转 m，保留原坐标原点 |
| `sample_points` / `voxel_size` | `3000` / `0.005` | 表面采样数、体素尺寸 m；先转换单位再下采样 |
| `min_depth` / `max_depth` | `0.2` / `2.0` | 彩色光学系 Z 深度范围 m |
| `bbox_shrink_ratio` | `0.0` | 框宽高收缩比例，背景过多时可试 0.1 |
| `sor_neighbors` / `sor_std_ratio` | `20` / `1.5` | 统计离群点过滤；neighbors=0 禁用 |
| `min_points` | `80` | 目标点数及 ICP 有效匹配对下限 |
| `initial_rpy_deg` | `[0,0,0]` | 模型在观测云坐标系中的初始 xyz 欧拉角，度 |
| `max_corr_dist` | `0.03` | ICP 对应点距离上限 m，不是相机量程 |
| `max_icp_iter` | `100` | ICP 最大迭代次数 |
| `fitness_threshold` | `0.3` | Open3D 模型点匹配比例下限 |
| `max_inlier_rmse` | `0.015` | 有效对应点残差上限 m |
| `sync_queue_size` / `sync_slop` | `30` / `0.05` | 同步缓存条数／采集时间差上限 s |
| `max_data_age` | `1.5` | 输入最大年龄及断流清空等待时间 s |

当前是点到点局部 ICP，平移以质心差初始化，旋转使用 `initial_rpy_deg`。没有全局粗配准、可见面筛选或对称消歧，初始朝向差太大时可能落入错误解。fitness 与 RMSE 只描述几何拟合，不是检测概率或真实位姿精度。算法接口见 [Open3D 官方说明](https://www.open3d.org/docs/release/python_api/open3d.pipelines.registration.registration_icp.html)。

## 验证

在 Engineer_27 根目录，不占用相机、不启动串口或机械臂：

```bash
source ros2_ws/install/setup.bash
ROS_DOMAIN_ID=198 PYTHONNOUSERSITE=1 OMP_NUM_THREADS=2 \
  "$CONDA_EXE" run --no-capture-output -n engineer python \
  ros2_ws/src/vision_nodes/test/test_pointcloud_pipeline.py -v
```

也可以直接用 engineer 环境的 Python 路径运行该文件。六项测试覆盖投影外参、无效深度、RGB 与大端／行填充转换、模型尺寸和原点、已知刚体变换回验、真实 ROS 近似配对与 TF、空检测、缺失 TF、过期数据及断流清空。系统 colcon 没有 Open3D 时会明确跳过该测试模块，不能把跳过视为 ICP 通过。

实物验收需要另行观察：框内是否包含矿的有效表面、模型叠合是否正确、不同朝向和遮挡下是否稳定。当前自动验证不证明这些指标已经达标。
