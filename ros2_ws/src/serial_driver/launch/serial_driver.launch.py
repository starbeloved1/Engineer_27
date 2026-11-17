"""共用的 ros2_control 启动；真实／模拟硬件由 serial_driver.yaml 决定。"""
import math
from pathlib import Path
import xacro
import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, LogInfo, OpaqueFunction, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def launch_setup(context):
    share = get_package_share_directory('serial_driver')
    path = Path(LaunchConfiguration('hardware_config').perform(context)).resolve()
    config = yaml.safe_load(path.read_text())
    if not isinstance(config, dict) or not isinstance(config.get('use_mock_hardware'), bool):
        raise ValueError(f'{path}: use_mock_hardware 必须为 true/false')
    for key, size in [('joint_min_degrees', 6), ('joint_max_degrees', 6), ('mock_initial_positions', 7)]:
        values = config.get(key)
        if not isinstance(values, list) or len(values) != size or any(
                isinstance(v, bool) or not isinstance(v, (int, float)) or not math.isfinite(v) for v in values):
            raise ValueError(f'{path}: {key} 须包含 {size} 个有限数值')
    description = {'robot_description': xacro.process_file(
        share + '/config/arm.ros2_control.xacro', mappings={'hardware_config': str(path)}).toxml()}
    manager = Node(package='controller_manager', executable='ros2_control_node',
                   parameters=[description, LaunchConfiguration('controller_config')], output='screen')
    actions = [
        LogInfo(msg=f"serial_driver: hardware={'mock' if config['use_mock_hardware'] else 'serial'}, config={path}"),
        Node(package='tf2_ros', executable='static_transform_publisher', name='world_to_base',
             arguments=['--frame-id', 'world', '--child-frame-id', 'base_link']),
        Node(package='robot_state_publisher', executable='robot_state_publisher', parameters=[description], output='screen'),
        manager,
        RegisterEventHandler(OnProcessExit(target_action=manager,
            on_exit=[EmitEvent(event=Shutdown(reason='controller_manager 已退出'))])),
        Node(package='controller_manager', executable='spawner',
             arguments=['joint_state_broadcaster', '--controller-manager-timeout', '30'], output='screen'),
        # 先加载成 inactive，监督节点只在新鲜 mode=2 时激活；退出该模式时停用以清除旧轨迹。
        Node(package='controller_manager', executable='spawner',
             arguments=['arm_controller', 'gripper_controller', '--inactive', '--controller-manager-timeout', '30'], output='screen'),
        Node(package='serial_driver', executable='hardware_supervisor',
             parameters=[{'status_timeout': config['receive_timeout_ms'] / 1000.0}], output='screen'),
    ]
    return actions


def generate_launch_description():
    share = get_package_share_directory('serial_driver')
    return LaunchDescription([
        DeclareLaunchArgument('hardware_config', default_value=share + '/config/serial_driver.yaml'),
        DeclareLaunchArgument('controller_config', default_value=share + '/config/ros2_controllers.yaml'),
        OpaqueFunction(function=launch_setup),
    ])
