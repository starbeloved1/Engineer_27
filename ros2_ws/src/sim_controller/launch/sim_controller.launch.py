"""启动模拟控制、状态反馈和 TF，按本包配置选择是否打开 RViz。"""
from pathlib import Path

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo, OpaqueFunction
from launch.substitutions import Command, FindExecutable, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from sim_controller.launch_support import rviz_node


def launch_setup(context):
    share = get_package_share_directory('sim_controller')
    path = Path(LaunchConfiguration('sim_config').perform(context))
    config = yaml.safe_load(path.read_text())
    if not isinstance(config, dict) or not isinstance(config.get('use_rviz'), bool):
        raise ValueError(f'{path}: use_rviz 必须为 YAML 布尔值 true 或 false')
    override = LaunchConfiguration('use_rviz').perform(context)
    if override not in ('', 'true', 'false'):
        raise ValueError('use_rviz 启动参数必须为 true 或 false')
    use_rviz = config['use_rviz'] if not override else override == 'true'
    description = {'robot_description': ParameterValue(Command([
        FindExecutable(name='xacro'), ' ', share + '/config/arm.ros2_control.xacro']), value_type=str)}
    actions = [
        LogInfo(msg=f'sim_controller: use_rviz={str(use_rviz).lower()}'),
        Node(package='tf2_ros', executable='static_transform_publisher', name='world_to_base',
             arguments=['--frame-id', 'world', '--child-frame-id', 'base_link']),
        Node(package='robot_state_publisher', executable='robot_state_publisher', parameters=[description], output='screen'),
        Node(package='controller_manager', executable='ros2_control_node',
             parameters=[description, LaunchConfiguration('controller_config')], output='screen'),
        Node(package='controller_manager', executable='spawner',
             arguments=['joint_state_broadcaster', '--controller-manager-timeout', '30']),
        Node(package='controller_manager', executable='spawner',
             arguments=['arm_controller', '--controller-manager-timeout', '30']),
    ]
    actions.append(Node(package='controller_manager', executable='spawner',
                        arguments=['gripper_controller', '--controller-manager-timeout', '30']))
    if config.get('show_energy', True):
        actions.append(Node(package='sim_controller', executable='energy_display_node', output='screen'))
    if use_rviz:
        actions.append(rviz_node())
    return actions


def generate_launch_description():
    share = get_package_share_directory('sim_controller')
    return LaunchDescription([
        DeclareLaunchArgument('controller_config', default_value=share + '/config/ros2_controllers.yaml'),
        DeclareLaunchArgument('sim_config', default_value=share + '/config/sim_controller.yaml'),
        DeclareLaunchArgument('use_rviz', default_value='', description='留空使用 sim_controller.yaml'),
        OpaqueFunction(function=launch_setup),
    ])
