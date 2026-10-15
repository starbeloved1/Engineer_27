"""只组合各包 launch：模拟控制、MoveIt、自研规划、Topic 输入和 RViz。"""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def include(package, filename, *, condition=None, arguments=None):
    return IncludeLaunchDescription(
        PythonLaunchDescriptionSource(get_package_share_directory(package) + '/launch/' + filename),
        condition=condition, launch_arguments=(arguments or {}).items())


def generate_launch_description():
    planning = get_package_share_directory('arm_planning')
    goal = get_package_share_directory('arm_goal')
    sim = get_package_share_directory('sim_controller')
    return LaunchDescription([
        DeclareLaunchArgument('use_rviz', default_value='true', description='启动只显示的 RViz'),
        DeclareLaunchArgument('start_planner', default_value='true'),
        DeclareLaunchArgument('start_goal', default_value='true', description='启动常驻 Topic 目标客户端'),
        DeclareLaunchArgument('planner_config', default_value=planning + '/config/arm_planning.yaml'),
        DeclareLaunchArgument('goal_config', default_value=goal + '/config/arm_goal.yaml'),
        DeclareLaunchArgument('controller_config', default_value=sim + '/config/ros2_controllers.yaml'),
        Node(package='tf2_ros', executable='static_transform_publisher',
             arguments=['--frame-id', 'world', '--child-frame-id', 'base_link']),
        include('sim_controller', 'sim_controller.launch.py',
                arguments={'controller_config': LaunchConfiguration('controller_config')}),
        include('moveit_config', 'moveit.launch.py'),
        include('arm_planning', 'arm_planning.launch.py',
                condition=IfCondition(LaunchConfiguration('start_planner')),
                arguments={'planner_config': LaunchConfiguration('planner_config')}),
        include('arm_goal', 'arm_goal.launch.py',
                condition=IfCondition(LaunchConfiguration('start_goal')),
                arguments={'goal_config': LaunchConfiguration('goal_config')}),
        include('eng_bringup', 'rviz.launch.py', condition=IfCondition(LaunchConfiguration('use_rviz'))),
    ])
