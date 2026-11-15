from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    camera_share = Path(get_package_share_directory('camera_driver'))
    node = Node(package='video_recorder', executable='video_recorder_node', name='video_recorder',
                output='screen', parameters=[LaunchConfiguration('camera_config'), {
                    'camera_config': LaunchConfiguration('camera_config'),
                }])
    return LaunchDescription([
        DeclareLaunchArgument('camera_config', default_value=str(camera_share / 'config/camera.yaml')),
        RegisterEventHandler(OnProcessExit(target_action=node,
            on_exit=[EmitEvent(event=Shutdown(reason='RGB recorder exited'))])),
        node,
    ])
