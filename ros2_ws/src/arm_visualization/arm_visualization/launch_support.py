"""集中加载 MoveIt 参数与本机 Humble RViz 插件兼容设置"""
import os
from pathlib import Path

from ament_index_python.packages import get_package_prefix
from moveit_config.configuration import load_moveit_config as moveit_config


def rviz_environment(native=False):
    # Humble 的显示插件卸载早于共享 ROS node 析构时可触发 CallbackGroup 析构崩溃
    # 只在 RViz 进程中预加载插件，使其代码在 node 清理期间仍保持有效
    directory = Path(get_package_prefix('moveit_ros_visualization')) / 'lib'
    plugins = ['libmoveit_trajectory_rviz_plugin.so', 'libmoveit_planning_scene_rviz_plugin.so']
    if native:
        plugins.append('libmoveit_motion_planning_rviz_plugin.so')
    values = [os.environ.get('LD_PRELOAD', ''), *(str(directory / name) for name in plugins)]
    return {'LD_PRELOAD': ':'.join(value for value in values if value)}
