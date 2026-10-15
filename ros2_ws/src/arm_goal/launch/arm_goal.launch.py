"""常驻 Topic 输入节点；CLI 单次输入使用 ros2 run arm_goal plan_goal。"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('goal_config', default_value=get_package_share_directory('arm_goal') + '/config/arm_goal.yaml'),
        Node(package='arm_goal', executable='plan_goal', arguments=['--listen'],
             parameters=[LaunchConfiguration('goal_config')], output='screen'),
    ])
