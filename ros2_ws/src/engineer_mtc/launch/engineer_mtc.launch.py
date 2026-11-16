"""只启动 MTC 任务节点；机器人状态与 move_group 由 run.sh 组织。"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from moveit_config.configuration import load_moveit_config


def generate_launch_description():
    config = load_moveit_config()
    default = get_package_share_directory('engineer_mtc') + '/config/engineer_mtc.yaml'
    return LaunchDescription([
        DeclareLaunchArgument('mtc_config', default_value=default),
        Node(package='engineer_mtc', executable='engineer_mtc_node', name='engineer_mtc',
             parameters=[config.to_dict(), default, LaunchConfiguration('mtc_config')],
             output='screen'),
    ])
