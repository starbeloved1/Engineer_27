# vision_nodes

## 基本描述

基于 RealSense D435 的能量单元识别与位姿解算

检测和录像的 Python 节点默认使用当前 `VIRTUAL_ENV`，未激活环境时使用 `~/venv`；
`yolo_python:=FILE` 可单独覆盖检测节点的解释器。构建和 ROS launch 使用系统 Python。
在 Jetson 上先准备与 JetPack 匹配的 NVIDIA PyTorch / torchvision，再运行
`~/venv/bin/python3 -m pip install -r requirements.txt`（工程根目录）安装 Python 依赖。

原理：RealSense 获取彩色图像和点云 → YOLO 或手动框选能量单元 → 保留框内点云 → SOR 去离群点、体素降采样 → 与模型进行 ICP 配准 → 位姿后处理 → 输出矿的位置和朝向。

`vision_nodes/` 是功能分组目录，包含以下独立 ROS 包：

1. **相机驱动 `camera_driver`**：启动官方 `realsense2_camera`，发布图像、内参、点云和相机 TF，同时提供 Foxglove Bridge 和 RViz 启动配置。
2. **检测节点 `yolo_detector`**：订阅彩色图像，发布 YOLO 检测框或固定手动 ROI，并在同帧原图上绘制框。
3. **模型发布节点 `model_cloud_publisher`**：读取 `assets/energy/energy.ply`，缩放为米、随机限点、体素降采样，发布 ICP 源点云。源码位于 `cloud_publisher/`。
4. **配准节点 `icp_registration`**：使用内参和 TF 将点云投影到彩色图像，按框提取观测云，过滤后执行 PCL ICP，验收并修正位姿。
5. **录像节点 `video_recorder`**：只录制相机彩色原图，保存到工程根目录的 `video/`，格式为 MP4。
6. **公共库 `vision_core`**：点云读取、过滤、ICP 和位姿约束的 C++ 实现

## 话题与数据流

### 相机输入

- `/camera/camera/color/image_raw`（`sensor_msgs/msg/Image`）：D435 彩色原图，供检测、叠框显示和录像使用。
- `/camera/camera/color/camera_info`（`sensor_msgs/msg/CameraInfo`）：彩色相机内参和光学坐标系，供点云投影使用。
- `/camera/camera/depth/color/points`（`sensor_msgs/msg/PointCloud2`）：D435 原始点云。

### 检测与模型输出

- `/vision/yolo/detections`（`vision_msgs/msg/Detection2DArray`）：yolo 检测框
- `/vision/visualization/image`（`sensor_msgs/msg/Image`）：同帧彩色图像叠加检测框
- `/vision/visualization/image/compressed`（`sensor_msgs/msg/CompressedImage`）：按需发布的 JPEG 叠框图，推荐远程 Foxglove Image 面板使用；质量由 `visualization_jpeg_quality` 配置。
- `/vision/model_cloud`（`sensor_msgs/msg/PointCloud2`）：模型源点云，供 ICP 配准

### ICP 输出

`/vision/icp/object_poses`（`geometry_msgs/msg/PoseArray`）是主要业务输出，表示后处理后的矿位姿，供 `engineer_mtc` 使用

`/vision/icp/raw_object_poses`（`geometry_msgs/msg/PoseArray`）是通过 ICP 验收、尚未进行位姿后处理的结果。

调试点云：
- `/vision/icp/roi_cloud`：框内观测云，已经过无效点和距离筛选，尚未执行 SOR、体素过滤。
- `/vision/icp/target_cloud`：SOR 和体素过滤后的观测云，即 ICP 目标云。
- `/vision/icp/raw_aligned_cloud`：通过验收的原始 ICP 配准模型。
- `/vision/icp/aligned_cloud`：按最终后处理位姿变换的模型。


## 配准与后处理

观测云先剔除无效点和超出 `max_range` 的点，再按检测框取点，依次执行 SOR 和体素过滤。模型云会单独执行缩放、随机限点和体素过滤。

ICP 使用point_to_point配准。首次用质心平移初始化，后续复用同序号检测上次成功的变换作为初值;通过配准验收后，按模型 Z 轴构造朝向，并根据配置进行六矿布局先验、Z 轴朝外和轴角约束修正。`raw_object_poses` 与 `object_poses` 可用于对比修正前后结果

## Foxglove 远程显示

Image 面板选择 `/vision/visualization/image/compressed`，取消原始叠框图订阅。
不需要的原始点云和深度图显示也应关闭：仅隐藏面板不一定取消订阅，需取消对应话题选择。
1280×720 RGB8 在 30 FPS 时原始数据约 83 MB/s；JPEG 可降低网络传输量，具体大小取决于画面。
压缩只在有订阅者时执行，检测、ICP 与录像仍使用原始数据。
