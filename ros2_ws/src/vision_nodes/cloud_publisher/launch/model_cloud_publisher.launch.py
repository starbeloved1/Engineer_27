from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    share = Path(get_package_share_directory('model_cloud_publisher'))
    node = Node(package='model_cloud_publisher', executable='model_cloud_publisher_node', name='model_cloud_publisher',
                output='screen', parameters=[LaunchConfiguration('vision_config')])
    return LaunchDescription([
        DeclareLaunchArgument('vision_config', default_value=str(share / 'config/model_cloud_publisher.yaml')),
        RegisterEventHandler(OnProcessExit(target_action=node,
            on_exit=[EmitEvent(event=Shutdown(reason='model_cloud_publisher exited'))])),
        node,
    ])
