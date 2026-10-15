"""仅启动模拟控制及状态反馈；不启动 MoveIt、规划业务或 RViz。"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, FindExecutable, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    share = get_package_share_directory('sim_controller')
    description = {'robot_description': ParameterValue(Command([
        FindExecutable(name='xacro'), ' ', share + '/config/arm.ros2_control.xacro']), value_type=str)}
    return LaunchDescription([
        DeclareLaunchArgument('controller_config', default_value=share + '/config/ros2_controllers.yaml'),
        Node(package='tf2_ros', executable='static_transform_publisher', name='world_to_base',
             arguments=['--frame-id', 'world', '--child-frame-id', 'base_link']),
        Node(package='robot_state_publisher', executable='robot_state_publisher', parameters=[description], output='screen'),
        Node(package='controller_manager', executable='ros2_control_node',
             parameters=[description, LaunchConfiguration('controller_config')], output='screen'),
        Node(package='controller_manager', executable='spawner',
             arguments=['joint_state_broadcaster', '--controller-manager-timeout', '30']),
        Node(package='controller_manager', executable='spawner',
             arguments=['arm_controller', '--controller-manager-timeout', '30']),
    ])
