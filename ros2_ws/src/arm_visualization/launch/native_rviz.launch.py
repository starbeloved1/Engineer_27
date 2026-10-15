"""可选原生调试入口：按钮直接调用 MoveIt，不经过 arm_planning"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from arm_visualization.launch_support import moveit_config, rviz_environment


def generate_launch_description():
    config = moveit_config()
    return LaunchDescription([Node(
        package='rviz2', executable='rviz2', name='rviz_native',
        arguments=['-d', get_package_share_directory('arm_visualization') + '/config/native_moveit.rviz'],
        parameters=[config.to_dict()], additional_env=rviz_environment(native=True), output='screen')])
