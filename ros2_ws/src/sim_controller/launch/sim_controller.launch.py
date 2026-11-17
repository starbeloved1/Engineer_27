"""组合 serial_driver 硬件控制与模型显示，按本包配置选择是否打开 RViz。"""
from pathlib import Path

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, LogInfo, OpaqueFunction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from sim_controller.launch_support import rviz_node


def launch_setup(context):
    path = Path(LaunchConfiguration('sim_config').perform(context))
    config = yaml.safe_load(path.read_text())
    if not isinstance(config, dict) or not isinstance(config.get('use_rviz'), bool):
        raise ValueError(f'{path}: use_rviz 必须为 YAML 布尔值 true 或 false')
    override = LaunchConfiguration('use_rviz').perform(context)
    if override not in ('', 'true', 'false'):
        raise ValueError('use_rviz 启动参数必须为 true 或 false')
    use_rviz = config['use_rviz'] if not override else override == 'true'
    hardware_share = get_package_share_directory('serial_driver')
    actions = [
        LogInfo(msg=f'sim_controller: use_rviz={str(use_rviz).lower()}'),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(
            hardware_share + '/launch/serial_driver.launch.py'), launch_arguments={
                'hardware_config': LaunchConfiguration('hardware_config'),
                'controller_config': LaunchConfiguration('controller_config'),
            }.items()),
    ]
    if config.get('show_energy', True):
        actions.append(Node(package='sim_controller', executable='energy_display_node', output='screen'))
    if use_rviz:
        actions.append(rviz_node())
    return actions


def generate_launch_description():
    share = get_package_share_directory('sim_controller')
    hardware_share = get_package_share_directory('serial_driver')
    return LaunchDescription([
        DeclareLaunchArgument('hardware_config', default_value=hardware_share + '/config/serial_driver.yaml'),
        DeclareLaunchArgument('controller_config', default_value=hardware_share + '/config/ros2_controllers.yaml'),
        DeclareLaunchArgument('sim_config', default_value=share + '/config/sim_controller.yaml'),
        DeclareLaunchArgument('use_rviz', default_value='', description='留空使用 sim_controller.yaml'),
        OpaqueFunction(function=launch_setup),
    ])
