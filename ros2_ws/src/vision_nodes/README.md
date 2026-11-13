# vision_nodes：取矿视觉节点

本包集中管理视觉节点，参考 `engineer_2026/jetson` 的 Python 包组织方式，以及 `Hero_27/heros` 的参数归属、按需发布和入口职责划分。默认使用 `Engineer_27/assets/best.pt`，权重复制自 26 的 `jetson` 分支，保留参考仓库原文件；运行时不依赖参考仓库的模型路径或 ROS 安装环境。

目前完成 D435 图像／点云采集、YOLO 二维检测和独立的可视化图像输出。原始深度继续由相机驱动发布，供后续定位使用。当前点云仍是整个场景，尚未提取框内目标点云或接入 ICP。

## 数据流

```text
D435 → 官方 RealSense 驱动 /camera/camera
         ├─ 彩色原图 ┬→ /yolo_detector → /vision/yolo/detections
         │          └──────────┐              │
         │                     ↓              ↓
         │                /main_visualizer（同帧配对、画框）
         │                     └→ /vision/visualization/image
         ├─ 原始深度、对齐深度、相机内参
         ├─ 彩色场景点云
         └─ 相机内部 TF
                    ↓ ROS 2 话题
             /foxglove_bridge
                    ↓ WebSocket
             Foxglove Image / 3D 面板
```

`yolo_detector` 只发布检测框数值，不绘图；`main_visualizer` 订阅原图和检测框，按相同的采集时间戳精确配对，在原图副本上画框、类别 ID 和置信度。点云由驱动直接提供，不重复经过可视化节点。

## 构建与启动

终端使用系统 ROS 2 Humble 环境，无需激活 Conda。launch 只为 `yolo_detector` 选择 Conda `engineer` 的 Python 3.10，其他节点继续使用系统 Python。检测环境包含 PyTorch 2.8.0+cpu、torchvision 0.23.0+cpu 和 `requirements-yolo.txt` 中锁定的依赖；当前默认使用 CPU。

本机依赖已安装。新环境复现时，先在 Conda `engineer` 中安装 CPU 版 PyTorch，再安装本包的推理依赖：

```bash
conda activate engineer
python -m pip install torch==2.8.0 torchvision==0.23.0 --index-url https://download.pytorch.org/whl/cpu
python -m pip install -r ros2_ws/src/vision_nodes/requirements-yolo.txt
conda deactivate
```

先关闭 RealSense Viewer 或旧的相机流水线，在 `Engineer_27` 根目录运行：

```bash
./build.sh --packages-select vision_nodes
./run.sh mode:=vision
```

这个入口启动 D435、`yolo_detector`、`main_visualizer` 和 Foxglove Bridge，不打开 RViz。`use_rviz` 仍用于原来的 `planning` 模式。权重不存在或加载失败时节点退出，不会自动下载其他模型来替代。

在 Foxglove 中选择 **Open connection → Foxglove WebSocket**，输入：

```text
ws://127.0.0.1:8765
```

添加两个面板：

| 面板 | 设置 |
| --- | --- |
| Image | Topic 选择 `/vision/visualization/image` |
| 3D | Display frame 选择 `camera_link`，启用 `/camera/camera/depth/color/points` |

也可以另加 Image 面板查看 `/camera/camera/color/image_raw`，与画框图对照。图中 `0: 0.90` 表示类别 ID 0、置信度 0.90；当前 26 权重的类别映射为 `0 → item`。点云包含整个场景；相机尚未标定到机器人基座。

`Ctrl+C` 停止采集、可视化节点和 Bridge；关闭 Foxglove 客户端只停止显示，不停止采集。本机连接默认仅监听 `127.0.0.1`，其他电脑连接时需在 Bridge YAML 中设置可访问的监听地址。

同一工作区、同一 ROS 域只允许启动一套视觉流程，重复运行 `./run.sh mode:=vision` 会直接提示并退出。换终端不会停止已有流程，需要先在原终端按 `Ctrl+C`。锁由系统在进程退出后自动释放，无需删除锁文件。

## 文件与回调

```text
vision_nodes/
├── vision_nodes/
│   ├── main_visualizer_node.py
│   ├── yolo_detector_node.py
│   └── detection_visualization.py
├── config/
│   ├── camera.yaml
│   ├── vision_params.yaml
│   └── foxglove_bridge.yaml
├── launch/
│   └── vision_pipeline.launch.py
├── test/
│   ├── test_visualization_pipeline.py
│   ├── test_visualizer_sync.py
│   └── test_detection_visualization.py
├── resource/vision_nodes
├── package.xml
├── setup.py
└── setup.cfg
```

- `YoloDetector.__init__`：读取参数、加载权重、建立原图订阅和检测结果发布器
- `YoloDetector.image_callback`：将原图转为 BGR、推理，将结果转换成 `Detection2DArray`；每个实际处理帧都发布结果，没有目标则发布空数组
- `MainVisualizer.__init__`：声明参数，建立原图／检测订阅、时间戳同步器和可视化发布器
- `MainVisualizer.image_callback`：检查是否有人订阅输出，按配置限频，将同帧检测交给绘制函数；输出为 BGR8，保留采集时间戳和 `frame_id`
- `draw_detections`：只在图像副本上画框和标签，不依赖 ROS 或 YOLO；过滤无效框并裁剪越界部分
- `main`：只做 ROS 初始化、创建节点、spin、异常处理和退出清理
- launch：加载包内配置，组织驱动、可视化节点和 Bridge；任一进程退出时停止同组进程
- `setup.py`：注册 `yolo_detector`、`main_visualizer` 入口并安装配置与 launch

参考 Hero 的按需发布方式，没有下游订阅 `/vision/visualization/image` 时不发布调试图。Foxglove 订阅该话题后，Bridge 会建立 ROS 订阅，触发调试图输出。算法节点应直接订阅原始数据，不依赖显示端是否打开。

推理和显示通过原图 `header.stamp` 配对，同时检查 `frame_id`。缓存满时旧帧被淘汰，不拿相邻帧的框补画。显示速度跟随实际检测输出；空数组会显示当前帧的干净原图，不残留上一帧的框。

## 接口

| 发布节点 | 话题 | 类型与含义 |
| --- | --- | --- |
| `/camera/camera` | `/camera/camera/color/image_raw` | `sensor_msgs/Image`，彩色原图 |
| `/camera/camera` | `/camera/camera/color/camera_info` | `sensor_msgs/CameraInfo`，相机内参与畸变 |
| `/camera/camera` | `/camera/camera/depth/image_rect_raw` | `sensor_msgs/Image`，`16UC1` 深度，单位 mm |
| `/camera/camera` | `/camera/camera/aligned_depth_to_color/image_raw` | `sensor_msgs/Image`，对齐到彩色像素的深度 |
| `/camera/camera` | `/camera/camera/depth/color/points` | `sensor_msgs/PointCloud2`，彩色场景点云，单位 m |
| `/camera/camera` | `/tf_static` | `tf2_msgs/TFMessage`，相机内部坐标关系 |
| `/yolo_detector` | `/vision/yolo/detections` | `vision_msgs/Detection2DArray`，二维检测框、类别 ID、置信度 |
| `/main_visualizer` | `/vision/visualization/image` | `sensor_msgs/Image`，BGR8 画框图；关闭检测时原样转发原图 |

检测数组和每个 `Detection2D` 的 header 保留原图采集时间与坐标系。`bbox.center.position.x/y`、`bbox.size_x/y` 为原始图像中的像素坐标与宽高；YOLO 输出轴对齐框，`bbox.center.theta=0`。`results[].hypothesis.class_id` 是字符串形式的类别索引，`score` 为置信度。这些消息不包含三维位置、物理尺寸或稳定目标身份。

图像订阅默认 Reliable，输出为 Reliable，队列只保留 1 帧，Durability 为 Volatile。本机此前实测 Best Effort 大图像存在丢帧，因此保留 Reliable；需要连接 Best Effort 图像源时可修改参数。

## 参数归属

`config/vision_params.yaml` 中的 `main_visualizer`：

| 参数 | 默认值 | 含义 |
| --- | --- | --- |
| `color_topic` | `/camera/camera/color/image_raw` | 订阅的彩色原图 |
| `visualization_topic` | `/vision/visualization/image` | 发布的调试图，不能与输入相同 |
| `detection_topic` | `/vision/yolo/detections` | 订阅的二维检测数组 |
| `draw_detections` | `true` | 单独运行节点时控制画框；流水线由 `start_detector` 联动覆盖 |
| `sync_queue_size` | `30` | 按时间戳缓存的帧数上限，过旧帧直接淘汰 |
| `visualization_rate_hz` | `0.0` | 0 跟随输入；正数限制最大发布频率，单位 Hz |
| `input_reliability` | `reliable` | 输入 QoS，可选 `reliable`、`best_effort` |

同一文件的 `yolo_detector`：

| 参数 | 默认值 | 含义 |
| --- | --- | --- |
| `model_path` | `assets/best.pt` | 本项目的权重文件，`run.sh` 将相对路径基准固定为 Engineer_27 根目录 |
| `color_topic` | `/camera/camera/color/image_raw` | 订阅彩色原图 |
| `detection_topic` | `/vision/yolo/detections` | 发布检测数组 |
| `conf_thres` | `0.5` | 最低置信度，范围 0～1 |
| `iou_thres` | `0.45` | NMS 的 IoU 阈值，范围 0～1 |
| `device` | `cpu` | 推理设备；使用 GPU 前需另外验证匹配的 CUDA PyTorch |
| `imgsz` | `640` | 推理缩放尺寸，32 的正整数倍，检测框仍对应原图像素 |
| `log_every_n` | `30` | 每多少个推理帧打印耗时；0 表示仅首帧打印 |
| `input_reliability` | `reliable` | 输入图像 QoS |

`config/camera.yaml` 归相机驱动，当前使用 USB 2 已验证的 640×480、6 FPS，并启用深度对齐、RGB 点云和相机 TF。`config/foxglove_bridge.yaml` 归 Bridge：`address`、`port` 控制监听地址与端口，`topic_whitelist` 允许相机／视觉／TF 话题，`min_qos_depth` 和 `max_qos_depth` 将显示端订阅队列限定为 1 帧。

已有相机或 Bridge 时，可以复用：

```bash
./run.sh mode:=vision start_camera:=false
./run.sh mode:=vision start_bridge:=false
# 暂时只看原图和点云：
./run.sh mode:=vision start_detector:=false
```

通过 `camera_config:=FILE`、`vision_config:=FILE`、`foxglove_config:=FILE` 覆盖所属配置。`yolo_python:=FILE` 可显式选择推理解释器，默认根据 `CONDA_EXE`／`conda` 可执行路径寻找 `engineer` 环境，找不到 Conda 时使用当前 launch 的 Python。也可以在 Engineer_27 根目录单独运行 `ros2 launch vision_nodes vision_pipeline.launch.py`；从其他目录启动时请配置绝对权重路径。

## 验证

```bash
# Engineer_27 根目录，合成图像／点云，不占用相机
source ros2_ws/install/setup.bash
cd ros2_ws
PYTHONNOUSERSITE=1 colcon test --packages-select vision_nodes --event-handlers console_direct+
colcon test-result --test-result-base build/vision_nodes --verbose
```

自动测试包括纯绘图边界、不同时间戳／坐标系拒绝配对、结果先到、空检测清框，以及相机基础话题到 Bridge 的合成链路。基础桥接测试关闭真实 YOLO，通过 Foxglove WebSocket 协议核对 12 对原图和输出。测试依赖 `python3-websocket`。

相机空闲时，在根目录额外验证真实 D435：

```bash
source ros2_ws/install/setup.bash
/usr/bin/python3 ros2_ws/src/vision_nodes/test/test_visualization_pipeline.py --camera
# 不开相机，使用合成图像运行真实权重：
/usr/bin/python3 ros2_ws/src/vision_nodes/test/test_visualization_pipeline.py --detector
# 相机和真实权重一起验证：
/usr/bin/python3 ros2_ws/src/vision_nodes/test/test_visualization_pipeline.py --camera --detector
```

带 `--detector` 的测试另外核对原图、检测数组和叠加图的同帧对应关系。链路验证不代表矿的识别率已经达标；模型效果需要用实物、距离、光照和遮挡进一步确认。
