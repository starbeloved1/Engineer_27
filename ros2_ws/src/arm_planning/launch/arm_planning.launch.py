"""仅启动自研规划 Action 服务端。"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from moveit_config.configuration import load_moveit_config


def generate_launch_description():
    config = load_moveit_config()
    return LaunchDescription([
        DeclareLaunchArgument('planner_config', default_value=get_package_share_directory('arm_planning') + '/config/arm_planning.yaml'),
        Node(package='arm_planning', executable='arm_planning_node',
             parameters=[config.robot_description, config.robot_description_semantic,
                         config.robot_description_kinematics, config.joint_limits,
                         LaunchConfiguration('planner_config')], output='screen'),
    ])
