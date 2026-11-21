"""集中加载 MoveIt 参数与本机 Humble RViz 插件兼容设置"""
import os
from pathlib import Path

from ament_index_python.packages import get_package_prefix, get_package_share_directory
from launch_ros.actions import Node
from moveit_config.configuration import load_moveit_config as moveit_config


def rviz_environment(native=False, mtc_details=False):
    # Humble 的显示插件卸载早于共享 ROS node 析构时可触发 CallbackGroup 析构崩溃
    # 只在 RViz 进程中预加载插件，使其代码在 node 清理期间仍保持有效
    directory = Path(get_package_prefix('moveit_ros_visualization')) / 'lib'
    plugins = ['libmoveit_trajectory_rviz_plugin.so', 'libmoveit_planning_scene_rviz_plugin.so']
    if native:
        plugins.append('libmoveit_motion_planning_rviz_plugin.so')
    libraries = [directory / name for name in plugins]
    if mtc_details:
        libraries.append(Path(get_package_prefix('moveit_task_constructor_visualization'))
                         / 'lib/libmotion_planning_tasks_rviz_plugin.so')
    values = [os.environ.get('LD_PRELOAD', ''), *(str(path) for path in libraries)]
    return {'LD_PRELOAD': ':'.join(value for value in values if value)}


def rviz_node(native=False, mtc_details=False):
    """创建显示节点；不启动控制器、状态发布器或规划节点。"""
    layout = 'mtc_remote.rviz' if mtc_details else ('native_moveit.rviz' if native else 'mtc.rviz')
    return Node(
        package='rviz2', executable='rviz2', name='rviz_native' if native else 'rviz_mtc',
        arguments=['-d', get_package_share_directory('sim_controller') + '/config/' + layout],
        parameters=[moveit_config().to_dict()], additional_env=rviz_environment(native, mtc_details),
        output='screen')
