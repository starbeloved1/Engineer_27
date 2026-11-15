"""启动 Foxglove Bridge，订阅和转发现有相机、视觉及 TF 数据。"""
from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    share = Path(get_package_share_directory('camera_driver'))
    bridge = Node(package='foxglove_bridge', executable='foxglove_bridge', name='foxglove_bridge',
                  output='screen', parameters=[LaunchConfiguration('foxglove_config')])
    return LaunchDescription([
        DeclareLaunchArgument('foxglove_config', default_value=str(share / 'config/foxglove_bridge.yaml')),
        RegisterEventHandler(OnProcessExit(target_action=bridge,
            on_exit=[EmitEvent(event=Shutdown(reason='Foxglove bridge exited'))])),
        bridge,
    ])
