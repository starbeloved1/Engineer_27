"""附加视觉 RViz，只订阅现有话题，不重复启动相机或视觉算法。"""
from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    share = Path(get_package_share_directory('vision_nodes_pcl'))
    return LaunchDescription([
        DeclareLaunchArgument('rviz_config', default_value=str(share / 'config/vision.rviz')),
        Node(package='rviz2', executable='rviz2', name='vision_rviz', output='screen',
             arguments=['-d', LaunchConfiguration('rviz_config')]),
    ])
