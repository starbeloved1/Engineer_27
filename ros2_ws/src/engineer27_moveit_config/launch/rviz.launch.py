"""Attach the planning UI to an already running offline demo in this ROS domain."""
from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    config = (MoveItConfigsBuilder('urdf', package_name='engineer27_moveit_config')
              .robot_description(file_path='config/arm.urdf.xacro')
              .robot_description_semantic(file_path='config/arm.srdf')
              .planning_pipelines(pipelines=['ompl'])
              .to_moveit_configs())
    return LaunchDescription([Node(
        package='rviz2', executable='rviz2',
        arguments=['-d', str(config.package_path / 'config/moveit.rviz')],
        parameters=[config.to_dict()], output='screen')])
