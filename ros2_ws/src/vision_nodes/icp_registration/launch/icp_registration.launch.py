from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    share = Path(get_package_share_directory('icp_registration'))
    node = Node(package='icp_registration', executable='icp_registration_node', name='icp_registration',
                output='screen', parameters=[LaunchConfiguration('vision_config')])
    return LaunchDescription([
        DeclareLaunchArgument('vision_config', default_value=str(share / 'config/icp_registration.yaml')),
        RegisterEventHandler(OnProcessExit(target_action=node,
            on_exit=[EmitEvent(event=Shutdown(reason='icp_registration exited'))])),
        node,
    ])
