"""加载纯机器人模型和 MoveIt 配置；不添加模拟硬件。"""
import os
from pathlib import Path
from ament_index_python.packages import get_package_prefix
from moveit_configs_utils import MoveItConfigsBuilder


def load_moveit_config():
    return (MoveItConfigsBuilder('urdf', package_name='moveit_config')
            .robot_description(file_path='config/arm.urdf.xacro')
            .robot_description_semantic(file_path='config/arm.srdf')
            .planning_pipelines(pipelines=['ompl']).to_moveit_configs())


def move_group_environment():
    # 本机 Humble 的插件卸载可早于共享节点 CallbackGroup 析构；只为 move_group
    # 保持这两个含 ROS 回调的插件加载，与 RViz 的进程级兼容处理相同，不修改系统库。
    plugins = [
        ('moveit_ros_move_group', 'libmoveit_move_group_default_capabilities.so'),
        ('moveit_simple_controller_manager', 'libmoveit_simple_controller_manager.so'),
    ]
    values = [os.environ.get('LD_PRELOAD', '')]
    values += [str(Path(get_package_prefix(package)) / 'lib' / library) for package, library in plugins]
    return {'LD_PRELOAD': ':'.join(value for value in values if value)}
