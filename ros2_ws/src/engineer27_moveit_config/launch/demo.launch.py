"""Offline planning demo: MoveIt + mock ros2_control + optional RViz/application."""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    config = (MoveItConfigsBuilder('urdf', package_name='engineer27_moveit_config')
              .robot_description(file_path='config/arm.urdf.xacro')
              .robot_description_semantic(file_path='config/arm.srdf')
              .planning_pipelines(pipelines=['ompl'])
              .to_moveit_configs())
    description = config.robot_description
    planner_parameters = [description, config.robot_description_semantic,
                          config.robot_description_kinematics, config.joint_limits]
    return LaunchDescription([
        DeclareLaunchArgument('use_rviz', default_value='true'),
        DeclareLaunchArgument('start_planner', default_value='true'),
        Node(package='tf2_ros', executable='static_transform_publisher',
             arguments=['--frame-id', 'world', '--child-frame-id', 'base_link']),
        Node(package='robot_state_publisher', executable='robot_state_publisher',
             parameters=[description], output='screen'),
        Node(package='controller_manager', executable='ros2_control_node',
             parameters=[description, str(config.package_path / 'config/ros2_controllers.yaml')], output='screen'),
        Node(package='controller_manager', executable='spawner',
             arguments=['joint_state_broadcaster', '--controller-manager-timeout', '30']),
        Node(package='controller_manager', executable='spawner',
             arguments=['arm_controller', '--controller-manager-timeout', '30']),
        Node(package='moveit_ros_move_group', executable='move_group',
             parameters=[config.to_dict(), {'publish_robot_description': True,
                 'publish_robot_description_semantic': True, 'publish_planning_scene': True,
                 'publish_geometry_updates': True, 'publish_state_updates': True,
                 'publish_transforms_updates': True, 'allow_trajectory_execution': True}], output='screen'),
        Node(package='arm_planning', executable='arm_planning_node',
             parameters=planner_parameters, output='screen',
             condition=IfCondition(LaunchConfiguration('start_planner'))),
        Node(package='rviz2', executable='rviz2',
             arguments=['-d', str(config.package_path / 'config/moveit.rviz')],
             parameters=[config.to_dict()], output='screen',
             condition=IfCondition(LaunchConfiguration('use_rviz'))),
    ])
