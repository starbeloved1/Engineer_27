# Engineer 27 robot description

本包从 `Engineer_27/assets/arm.urdf` 生成带 `package://engineer27_description/` 网格引用的 URDF。
只安装运行中的 visual/collision 网格，不安装 `assets/source`。原始模型文件保持不变。

在 `Engineer_27/ros2_ws` 中编译并加载：

```bash
source /opt/ros/humble/setup.bash
colcon build --symlink-install --packages-select engineer27_description
source install/setup.bash
```

在 MoveIt Setup Assistant 中导入：

```text
install/engineer27_description/share/engineer27_description/urdf/arm.urdf
```

网格引用按已加载的 ROS 包解析，不依赖启动助手时的工作目录。
修改 assets 中的 URDF 或重新生成网格后，重新执行编译命令。

规划组建议命名 `arm_control`，链为 `base_link` → `link_6`，包括全部 6 个旋转关节。
目前没有独立夹爪开合关节，也没有标定 TCP；先使用 `link_6` 为链末端。
当前关节为 continuous；真实角度限位应向机械、电控确认后更新。
配置包可以生成到 `ros2_ws/src/engineer27_moveit_config`。

package.xml 的维护者邮箱和 license 字段为占位值，请由项目维护者填写。
