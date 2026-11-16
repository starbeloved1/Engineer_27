# 模型工具

从工程根目录执行。预览不需要构建工作区，只需系统 ROS Humble。

```bash
source /opt/ros/humble/setup.bash
# 机械臂外观、关节滑块
ros2 launch tools/preview.launch.py
# 机械臂碰撞模型
ros2 launch tools/preview.launch.py rviz_config:="$PWD/tools/preview_collision.rviz"
# 单块能量单元
ros2 launch tools/preview.launch.py urdf:="$PWD/assets/energy/energy.urdf" \
  rviz_config:="$PWD/tools/energy.rviz" joint_states:=false
```

`preview.launch.py` 根据 URDF 根 link 设置 RViz 固定坐标系。相对网格路径以内存转换方式
解析为文件 URI，不修改源文件。它启动自己的模型发布器，应独立运行，避免和正在运行的
机械臂状态发布器混用 `/robot_description` 和同名 TF。

`build_energy_model.py` 新建能量单元 URDF 和覆盖报告，算法及坐标约定见
[模型说明](../assets/energy/README.md)。原有机械臂网格处理脚本仍在 `scripts/`。
