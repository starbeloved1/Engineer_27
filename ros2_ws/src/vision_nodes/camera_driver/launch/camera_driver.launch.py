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
    node = Node(package='realsense2_camera', executable='realsense2_camera_node',
                namespace='camera', name='camera', output='screen',
                parameters=[LaunchConfiguration('camera_config')])
    return LaunchDescription([
        DeclareLaunchArgument('camera_config', default_value=str(share / 'config/camera.yaml')),
        RegisterEventHandler(OnProcessExit(target_action=node,
            on_exit=[EmitEvent(event=Shutdown(reason='D435 driver exited'))])),
        node,
    ])
