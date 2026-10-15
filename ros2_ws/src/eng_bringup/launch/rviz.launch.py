"""附加只显示的 RViz，不重复启动规划服务和控制器"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from eng_bringup.launch_support import moveit_config, rviz_environment


def generate_launch_description():
    config = moveit_config()
    return LaunchDescription([Node(
        package='rviz2', executable='rviz2',
        arguments=['-d', get_package_share_directory('eng_bringup') + '/config/display.rviz'],
        parameters=[config.to_dict()], additional_env=rviz_environment(native=False), output='screen')])
