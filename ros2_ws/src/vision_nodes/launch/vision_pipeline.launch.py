"""启动 D435、YOLO、统一可视化图像输出和 Foxglove Bridge"""
import os
from pathlib import Path
import shutil
import sys

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    share = Path(get_package_share_directory('vision_nodes'))
    # 仅检测节点使用 engineer 环境，其余 ROS 节点继续使用系统 Python
    conda_executable = os.environ.get('CONDA_EXE') or shutil.which('conda')
    yolo_python = (str(Path(conda_executable).resolve().parents[1]
                       / 'envs/engineer/bin/python') if conda_executable else sys.executable)
    camera = Node(
        package='realsense2_camera', executable='realsense2_camera_node',
        namespace='camera', name='camera', output='screen',
        parameters=[LaunchConfiguration('camera_config')],
        condition=IfCondition(LaunchConfiguration('start_camera')),
    )
    detector = Node(
        package='vision_nodes', executable='yolo_detector', name='yolo_detector',
        output='screen', parameters=[LaunchConfiguration('vision_config')],
        prefix=[LaunchConfiguration('yolo_python')],
        additional_env={'PYTHONNOUSERSITE': '1'},
        condition=IfCondition(LaunchConfiguration('start_detector')),
    )
    visualizer = Node(
        package='vision_nodes', executable='main_visualizer', name='main_visualizer',
        output='screen', parameters=[LaunchConfiguration('vision_config'), {
            'draw_detections': ParameterValue(LaunchConfiguration('start_detector'), value_type=bool),
        }],
    )
    bridge = Node(
        package='foxglove_bridge', executable='foxglove_bridge', name='foxglove_bridge',
        output='screen', parameters=[LaunchConfiguration('foxglove_config')],
        condition=IfCondition(LaunchConfiguration('start_bridge')),
    )
    # 任一进程退出时停止同组进程，关闭 Foxglove 客户端不会停止采集
    handlers = [RegisterEventHandler(OnProcessExit(
        target_action=action,
        on_exit=[EmitEvent(event=Shutdown(reason=f'{label} exited'))],
    )) for action, label in [(camera, 'D435 driver'), (detector, 'YOLO detector'),
                             (visualizer, 'Main visualizer'),
                             (bridge, 'Foxglove bridge')]]
    return LaunchDescription([
        DeclareLaunchArgument('start_camera', default_value='true',
                              description='false 可接入已运行的相机或回放数据'),
        DeclareLaunchArgument('start_detector', default_value='true',
                              description='false 时仅显示原图，不运行 YOLO'),
        DeclareLaunchArgument('yolo_python', default_value=yolo_python,
                              description='安装了 Ultralytics 的 Python 3.10 可执行文件'),
        DeclareLaunchArgument('start_bridge', default_value='true',
                              description='已有 Bridge 时设为 false，避免端口冲突'),
        DeclareLaunchArgument('camera_config', default_value=str(share / 'config/camera.yaml')),
        DeclareLaunchArgument('vision_config', default_value=str(share / 'config/vision_params.yaml')),
        DeclareLaunchArgument('foxglove_config',
                              default_value=str(share / 'config/foxglove_bridge.yaml')),
        *handlers, camera, detector, visualizer, bridge,
    ])
