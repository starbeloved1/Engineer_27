import os
import shutil
import sys
from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    share = Path(get_package_share_directory('yolo_detector'))
    venv_python = Path(os.environ.get('VIRTUAL_ENV', str(Path.home() / 'venv'))) / 'bin/python3'
    python = str(venv_python) if venv_python.is_file() else sys.executable
    conda = os.environ.get('CONDA_EXE') or shutil.which('conda')
    if conda:
        engineer_python = Path(conda).resolve().parents[1] / 'envs/engineer/bin/python'
        if engineer_python.is_file():
            python = str(engineer_python)
    node = Node(package='yolo_detector', executable='yolo_detector_node', name='yolo_detector',
                output='screen', cwd=str(share), parameters=[LaunchConfiguration('vision_config')],
                prefix=[LaunchConfiguration('yolo_python')], additional_env={'PYTHONNOUSERSITE': '1'})
    return LaunchDescription([
        DeclareLaunchArgument('vision_config', default_value=str(share / 'config/yolo_detector.yaml')),
        DeclareLaunchArgument('yolo_python', default_value=python),
        RegisterEventHandler(OnProcessExit(target_action=node,
            on_exit=[EmitEvent(event=Shutdown(reason='YOLO detector exited'))])),
        node,
    ])
