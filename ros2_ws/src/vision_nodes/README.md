# vision_nodes：取矿视觉节点

本包集中管理视觉节点，参考 `engineer_2026/jetson` 的 Python 包组织方式，以及 `Hero_27/heros` 的参数归属、按需发布和入口职责划分。参考仓库只用于阅读，不是运行时依赖。

目前完成 D435 彩色图、原始深度、彩色场景点云，以及面向 Foxglove 的统一可视化输出。26 的 `vision_nodes` 没有独立深度伪彩图功能，本包已删除此前添加的 `depth_preview`、伪彩计算函数和视觉 RViz 配置。原始深度继续由相机驱动发布，供后续定位使用。

## 数据流

```text
D435 → 官方 RealSense 驱动 /camera/camera
         ├─ 彩色原图 → /main_visualizer → /vision/visualization/image
         ├─ 原始深度、对齐深度、相机内参
         ├─ 彩色场景点云
         └─ 相机内部 TF
                    ↓ ROS 2 话题
             /foxglove_bridge
                    ↓ WebSocket
             Foxglove Image / 3D 面板
```

`main_visualizer` 负责可视化图像输出，不负责采集或检测。当前尚未接 YOLO，输出与输入原图一致；以后在这里加入检测框等绘制逻辑。点云由驱动直接提供，不转成图片，也不重复经过可视化节点。

## 构建与启动

使用系统 ROS 2 Humble 环境，无需激活 Conda。依赖 `realsense2_camera` 和 `foxglove_bridge`；当前节点不需要 PyTorch、Open3D 或 OpenCV。

先关闭 RealSense Viewer 或旧的相机流水线，在 `Engineer_27` 根目录运行：

```bash
./build.sh --packages-select vision_nodes
./run.sh mode:=vision
```

这个入口只启动 D435、`main_visualizer` 和 Foxglove Bridge，不打开 RViz。`use_rviz` 仍用于原来的 `planning` 模式。

在 Foxglove 中选择 **Open connection → Foxglove WebSocket**，输入：

```text
ws://127.0.0.1:8765
```

添加两个面板：

| 面板 | 设置 |
| --- | --- |
| Image | Topic 选择 `/vision/visualization/image` |
| 3D | Display frame 选择 `camera_link`，启用 `/camera/camera/depth/color/points` |

也可以另加 Image 面板查看 `/camera/camera/color/image_raw`，与调试输出对照。当前没有识别结果，点云包含整个场景；相机尚未标定到机器人基座。

`Ctrl+C` 停止采集、可视化节点和 Bridge；关闭 Foxglove 客户端只停止显示，不停止采集。本机连接默认仅监听 `127.0.0.1`，其他电脑连接时需在 Bridge YAML 中设置可访问的监听地址。

同一工作区、同一 ROS 域只允许启动一套视觉流程，重复运行 `./run.sh mode:=vision` 会直接提示并退出。换终端不会停止已有流程，需要先在原终端按 `Ctrl+C`。锁由系统在进程退出后自动释放，无需删除锁文件。

## 文件与回调

```text
vision_nodes/
├── vision_nodes/
│   └── main_visualizer_node.py
├── config/
│   ├── camera.yaml
│   ├── vision_params.yaml
│   └── foxglove_bridge.yaml
├── launch/
│   └── vision_pipeline.launch.py
├── test/
│   └── test_visualization_pipeline.py
├── resource/vision_nodes
├── package.xml
├── setup.py
└── setup.cfg
```

- `MainVisualizer.__init__`：声明参数，建立图像订阅和可视化发布器
- `image_callback`：收到彩色图后检查是否有人订阅输出，按配置限频并发布；保留原始像素、编码、采集时间戳和 `frame_id`
- `main`：只做 ROS 初始化、创建节点、spin、异常处理和退出清理
- launch：加载包内配置，组织驱动、可视化节点和 Bridge；任一进程退出时停止同组进程
- `setup.py`：注册 `main_visualizer` 可执行入口并安装配置与 launch

参考 Hero 的按需发布方式，没有下游订阅 `/vision/visualization/image` 时不发布调试图。Foxglove 订阅该话题后，Bridge 会建立 ROS 订阅，触发调试图输出。算法节点应直接订阅原始数据，不依赖显示端是否打开。

## 接口

| 发布节点 | 话题 | 类型与含义 |
| --- | --- | --- |
| `/camera/camera` | `/camera/camera/color/image_raw` | `sensor_msgs/Image`，彩色原图 |
| `/camera/camera` | `/camera/camera/color/camera_info` | `sensor_msgs/CameraInfo`，相机内参与畸变 |
| `/camera/camera` | `/camera/camera/depth/image_rect_raw` | `sensor_msgs/Image`，`16UC1` 深度，单位 mm |
| `/camera/camera` | `/camera/camera/aligned_depth_to_color/image_raw` | `sensor_msgs/Image`，对齐到彩色像素的深度 |
| `/camera/camera` | `/camera/camera/depth/color/points` | `sensor_msgs/PointCloud2`，彩色场景点云，单位 m |
| `/camera/camera` | `/tf_static` | `tf2_msgs/TFMessage`，相机内部坐标关系 |
| `/main_visualizer` | `/vision/visualization/image` | `sensor_msgs/Image`，统一调试图；当前跟随原图编码 |

图像订阅默认 Reliable，输出为 Reliable，队列只保留 1 帧，Durability 为 Volatile。本机此前实测 Best Effort 大图像存在丢帧，因此保留 Reliable；需要连接 Best Effort 图像源时可修改参数。

## 参数归属

`config/vision_params.yaml` 中的 `main_visualizer`：

| 参数 | 默认值 | 含义 |
| --- | --- | --- |
| `color_topic` | `/camera/camera/color/image_raw` | 订阅的彩色原图 |
| `visualization_topic` | `/vision/visualization/image` | 发布的调试图，不能与输入相同 |
| `visualization_rate_hz` | `0.0` | 0 跟随输入；正数限制最大发布频率，单位 Hz |
| `input_reliability` | `reliable` | 输入 QoS，可选 `reliable`、`best_effort` |

`config/camera.yaml` 归相机驱动，当前使用 USB 2 已验证的 640×480、6 FPS，并启用深度对齐、RGB 点云和相机 TF。`config/foxglove_bridge.yaml` 归 Bridge：`address`、`port` 控制监听地址与端口，`topic_whitelist` 允许相机／视觉／TF 话题，`min_qos_depth` 和 `max_qos_depth` 将显示端订阅队列限定为 1 帧。

已有相机或 Bridge 时，可以复用：

```bash
./run.sh mode:=vision start_camera:=false
./run.sh mode:=vision start_bridge:=false
```

通过 `camera_config:=FILE`、`vision_config:=FILE`、`foxglove_config:=FILE` 覆盖所属配置；也可以单独运行 `ros2 launch vision_nodes vision_pipeline.launch.py`。

## 验证

```bash
# Engineer_27 根目录，合成图像／点云，不占用相机
source ros2_ws/install/setup.bash
cd ros2_ws
PYTHONNOUSERSITE=1 colcon test --packages-select vision_nodes --event-handlers console_direct+
colcon test-result --test-result-base build/vision_nodes --verbose
```

测试启动独立 ROS 域和临时 Bridge 端口，通过真实 Foxglove WebSocket 协议订阅并解码 CDR 数据，核对 12 对原图和可视化输出的时间戳、坐标系、编码、尺寸和像素，接收点云与 TF，并验证统一退出。测试依赖 `python3-websocket`。

相机空闲时，在根目录额外验证真实 D435：

```bash
source ros2_ws/install/setup.bash
/usr/bin/python3 ros2_ws/src/vision_nodes/test/test_visualization_pipeline.py --camera
```

下一步接入 26 的 YOLO 权重和检测节点，再将对应原图的检测框加入 `main_visualizer`；当前不发布虚构的检测或位姿结果。
