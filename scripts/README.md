# 网格工具的 Conda 环境

网格减面、碰撞生成和 FCL 检查使用 `engineer27-mesh` Conda 环境。
环境用来隔离这些 Python 工具的依赖，与 ROS/MoveIt 的系统环境分开。

## 创建与使用

在 `Engineer_27` 目录创建环境：

```bash
conda env create -f environment.yml
```

使用现有环境：

```bash
conda activate engineer27-mesh
python scripts/simplify_visual_meshes.py
python scripts/build_collision_meshes.py
python scripts/validate_collision_meshes.py
```

也可以直接执行，无需激活：

```bash
conda run -n engineer27-mesh python scripts/simplify_visual_meshes.py
```

依赖版本固定在 `mesh_requirements.txt`。更新已有环境的依赖：

```bash
conda run -n engineer27-mesh python -m pip install -r scripts/mesh_requirements.txt
```

网格生成读取 `assets/source/`，更新 `assets/engineer/` 中的运行模型；生成之前会校验原始资料归档。
从资产更新 ROS 描述包时，需要重新编译 `ros2_ws`。

## 验证

```bash
OPENBLAS_NUM_THREADS=1 conda run -n engineer27-mesh python -m unittest discover -s scripts -p 'test_*mesh*.py' -v
```

## ROS 与 MoveIt

ROS Humble 使用 Ubuntu 的系统 Python。运行 RViz、MoveIt 或 colcon 时退出网格环境：

```bash
conda deactivate
source /opt/ros/humble/setup.bash
source ros2_ws/install/setup.bash
ros2 launch moveit_setup_assistant setup_assistant.launch.py
```

脚本顶部的 `#!/usr/bin/env python3` 会使用当前激活环境的 Python；无需修改 Python 路径。
