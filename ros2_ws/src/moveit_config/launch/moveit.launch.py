"""仅启动 MoveIt 服务；需要外部提供关节状态和 TF。"""
from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_config.configuration import load_moveit_config, move_group_environment


def generate_launch_description():
    config = load_moveit_config()
    return LaunchDescription([Node(
        package='moveit_ros_move_group', executable='move_group', additional_env=move_group_environment(),
        parameters=[config.to_dict(), {'publish_robot_description': True,
            'publish_robot_description_semantic': True, 'publish_planning_scene': True,
            'publish_geometry_updates': True, 'publish_state_updates': True,
            'publish_transforms_updates': True, 'allow_trajectory_execution': True,
            'capabilities': 'move_group/ExecuteTaskSolutionCapability'}], output='screen')])
