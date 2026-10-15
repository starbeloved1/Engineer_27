from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    return LaunchDescription([Node(package='moveit_setup_assistant', executable='moveit_setup_assistant',
        arguments=['--config_pkg=' + get_package_share_directory('engineer27_moveit_config')], output='screen')])
