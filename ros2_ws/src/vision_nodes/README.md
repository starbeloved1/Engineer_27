# vision_nodes：取矿视觉功能分组

目录组织参考 `Hero_27/heros/ros2_ws/src/hero_aim`：`vision_nodes` 是分组目录，
自身不含 `package.xml` 或 `CMakeLists.txt`，下面各功能包由 colcon 独立发现、按依赖构建。
算法统一使用 PCL，没有另一套配准后端。视觉结果尚未接机械臂。

```text
vision_nodes/
├── vision_core/             共用 PCL 计算、位姿约束和节点启动工具，导出 C++ 库
├── camera_driver/           D435 官方驱动、Foxglove 和 RViz 配置与启动
├── yolo_detector/           Python YOLO、固定手动 ROI、同帧叠框图像
├── icp_registration/        C++ 点云 ROI、ICP 调度、后处理和位姿输出
├── cloud_publisher/         C++ PLY 模型点云发布（ROS 包 model_cloud_publisher）
└── video_recorder/          Python RGB 原图录像
```

每个 C++ 包各有 `package.xml`、`CMakeLists.txt`、`include/<包名>/`、`src/`；
节点入口为 `src/main.cpp`。各运行包拥有自己的 `config/`、`launch/`。
Python 包使用 `setup.py`、`setup.cfg`、`resource/`、同名 Python 模块目录及 `test/`。
`vision_core` 与 heros 的 `aim_core` 一样导出库，由节点包 `find_package(vision_core)` 后链接。

## 构建与运行

在 Engineer_27 根目录、未激活 Conda 的系统 ROS 终端运行：

```bash
./build.sh
./run.sh mode:=vision use_rviz:=true
```

只构建本分组时可用 `./build.sh --base-paths src/vision_nodes --executor sequential`。
`mode:=vision_pcl` 保留为 `mode:=vision` 的兼容别名。
总启动列表只维护在根目录 `run.sh`，分别启动各功能包的 launch；没有分组级 ROS 包或总 launch。
任一被监控的 launch 退出时统一停止其余流程，Ctrl+C 正常退出。

| 包 | 独立启动 | 配置归属 |
| --- | --- | --- |
| `camera_driver` | `ros2 launch camera_driver camera_driver.launch.py` | `camera_driver/config/camera.yaml` |
| `yolo_detector` | `ros2 launch yolo_detector yolo_detector.launch.py` | `yolo_detector/config/yolo_detector.yaml` |
| `model_cloud_publisher` | `ros2 launch model_cloud_publisher model_cloud_publisher.launch.py` | `cloud_publisher/config/model_cloud_publisher.yaml` |
| `icp_registration` | `ros2 launch icp_registration icp_registration.launch.py` | `icp_registration/config/icp_registration.yaml` |
| `video_recorder` | `ros2 launch video_recorder video_recorder.launch.py` | 与相机共用 `camera_driver/config/camera.yaml` 中 `/video_recorder` 段 |

单独运行可执行文件时名称为 `<包名>_node`，例如
`ros2 run icp_registration icp_registration_node --ros-args --params-file FILE`。
正常使用各包 launch 自动加载本包配置；节点本身的 ROS 名称和已有话题保持不变。

`run.sh` 保留 `start_camera`、`start_detector`、`start_icp`、`start_bridge`、`use_rviz` 开关。
`start_detector=false` 关闭检测、叠框图像、模型发布和 ICP；查看相机原图 `/camera/camera/color/image_raw`。
相机关闭时录像节点仍可订阅已有相机或回放的 RGB。
`camera_config:=FILE` 同时交给相机和录像；`foxglove_config:=FILE` 交给 camera_driver 的 Bridge 启动；
`yolo_python:=FILE` 指定推理解释器。
旧的 `vision_config:=FILE` 仍支持多节点 YAML 覆盖；平时直接编辑各包自己的配置。

## 相机与 YOLO

D435 默认 USB 3：彩色 `1280x720x30`、原始深度 `848x480x30`，RGB8/Z16，
保留帧同步、深度对齐、彩色点云和内部 TF。26 的 `start.md` 明确指定彩色 1280×720@30 和开启点云，
未显式指定深度档位；当前深度档位取本机 SDK 枚举的默认值。相机请求 30 FPS 不保证所有算法每帧处理。
修改流模式后需重启并重新选择手动 ROI。

基础 ROS 和录像使用系统 Python；YOLO launch 自动选择 Conda `engineer` 的 Python。
依赖清单为 `yolo_detector/requirements.txt`，模型权重保留在根目录 `model/best.pt`，
由 yolo_detector 安装到其 share/model。默认相对权重路径以该包 share 为工作目录解析。
`device: gpu` 选择第一张 CUDA GPU，`device: cpu` 选择 CPU，修改后重启节点；
节点内部将 gpu 转为推理库要求的设备编号 0。YOLO 结果保留原图像素坐标和采集时间戳。

YOLO 节点同时发布 `/vision/yolo/detections` 和 `/vision/visualization/image`。
叠框图使用本帧输入与实际发布的检测结果，包括手动 ROI；保留相同 header，BGR8 编码。
只在有订阅者时复制原图并绘制框、类别和分数，原始图像与 RGB 录像不被修改。
`visualization_topic`、`visualization_rate_hz` 归 `yolo_detector/config/yolo_detector.yaml`；
频率为 0 时跟随检测结果，正数只限制叠框图像，不限制检测框发布。空检测时发布无框图像。

`use_manual_roi: true` 跳过模型推理，鼠标拖框、松开后提交；后续每帧沿用这个矩形，
再次拖框完成才替换。拖动期间继续用旧框，关闭窗口不清空；分辨率或 frame_id 改变则作废。
Q 隐藏，Esc 取消当前拖动；再次显示窗口或切换模式：

```bash
ros2 service call /yolo_detector/select_roi std_srvs/srv/Trigger '{}'
ros2 param set /yolo_detector use_manual_roi false
```

## RGB 原图录像

开关仍在 `camera_driver/config/camera.yaml` 的 `/video_recorder.ros__parameters.video`。
只录 `/camera/camera/color/image_raw`，不含检测框、深度、点云或音频。
`fps=0` 从同一相机 YAML 的彩色 profile 读取帧率；关闭录制后取消订阅并完成 MP4 写入。

```bash
ros2 param set /video_recorder video true
ros2 param set /video_recorder video false
```

文件为 `Engineer_27/video/rgb_年月日_时分秒_微秒.mp4`，北京时间，mp4v 编码；
重启录制或分辨率变化时另建文件，不覆盖旧视频。目录不受启动 cwd 影响，已被 Git 忽略。

## PCL 流程和接口

模型文件为根目录 `assets/energy/energy.ply`，由 model_cloud_publisher 安装到本包 share/assets/energy。
保留 26 的点云和参数流程：PLY 顶点乘 0.001 → 最多 5000 点 → 5 mm 模型体素；
观测云距离筛选 → 按点云时间戳用 TF 转到彩色 CameraInfo 坐标系并投影框选 → SOR → 6 mm 体素 → 质心或上帧变换初始化 → 点到点 ICP。
ICP 最大 120 次、对应距离 1 m；`getFitnessScore()` 为 MSE（m²），
`metric=1/(1+MSE)`，并检查收敛。`max_fitness_score=0` 只关闭 MSE 上限。
数值以当前 YAML 为准。

原始 ICP 位姿通过后，最终输出依次经过只保留 Z 轴朝向、六矿槽位和朝向约束。
上一位姿缓存按检测数组下标，不是稳定目标 ID，YOLO 消失时不会独立跟踪。
两次检测消息间使用最近检测框；收到空检测后立即停止配准，不保留上一有效框来填补漏检。
ROI 投影使用 CameraInfo 的 frame_id 查询输入点云到彩色光学坐标系的 TF，
缺少变换或检测框 frame_id 不匹配时输出空结果。框内点仍按输入 XYZ 取出，再转到 target_frame，避免重复变换。
当前 D435 彩色 CameraInfo 的畸变系数为零，使用针孔投影；这里尚未实现非零畸变模型。
六矿约束依赖坐标系和旧布局假设，不代表已经验证适合 27 实物。

| 输入 | 类型 |
| --- | --- |
| `/camera/camera/color/image_raw` | Image |
| `/camera/camera/color/camera_info` | CameraInfo |
| `/camera/camera/depth/color/points` | PointCloud2 |
| `/vision/yolo/detections` | Detection2DArray |
| `/vision/model_cloud` | PointCloud2，模型独立 `energy_model` 坐标系 |

| 配准输出 | 含义 |
| --- | --- |
| `/vision/icp/roi_cloud` | 距离和 ROI 筛选后、SOR/体素前，RGB |
| `/vision/icp/target_cloud` | 过滤后的实际观测，配准失败也输出 |
| `/vision/icp/raw_aligned_cloud`、`raw_object_poses` | 原始完整 ICP 结果 |
| `/vision/icp/aligned_cloud`、`object_poses` | 最终约束后的模型和 PoseArray |

输出使用配置 `target_frame`，当前为 `camera_link`，保留点云采集时间，单位米。
原点是 PLY CAD 原点，不是夹爪 TCP；模型资料见 [能量单元模型说明](../../../assets/energy/README.md)。
无检测或失效时发布空结果，输入超过 `max_data_age` 会清空输出与缓存。
数组没有跨帧稳定身份；TF 历史不能撤销，后续业务应以当前 PoseArray 判断有效性。

## 观察与验证

`./run.sh mode:=vision use_rviz:=true` 打开固定相机视角 RViz。
已有流程时可单独执行 `ros2 launch camera_driver vision_rviz.launch.py`。
绿色是观测、橙色是原始 ICP、红色是最终约束模型；可在 Views 恢复相机、侧面、俯视方向。
Foxglove 地址 `ws://127.0.0.1:8765`，图像选 `/vision/visualization/image`，3D 参考系选 `camera_link`。
Bridge 参数和 RViz 布局在 `camera_driver/config/foxglove_bridge.yaml`、`vision.rviz`。
Bridge 由 `run.sh` 独立启动，也可单独运行 `ros2 launch camera_driver foxglove_bridge.launch.py`，
不依赖相机或 YOLO 是否由本入口启动。
原始 CAD 云无相机 TF，需要单独以 `energy_model` 参考系查看。

```bash
cd ros2_ws
source install/setup.bash
PYTHONNOUSERSITE=1 colcon test --packages-select yolo_detector video_recorder icp_registration --return-code-on-test-failure
colcon test-result --verbose
```

现有测试覆盖手动框持续复用、重选、图像几何改变、模式切换、同帧绘图及原图不被修改，以及录像开关、编码、尺寸变化和文件不覆盖。
ICP 的合成 ROS 消息测试检查跨相机旋转和平移后的 ROI 取点、输出坐标系、同坐标系直通、缺少 TF、框坐标系不匹配以及空检测清除旧框。
本次结构迁移另用独立 ROS 域、合成点云/RGB 回验跨包消息链和完整启动，不启动真实相机或机械臂。
