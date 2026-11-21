import os
import shutil
from pathlib import Path
import sys
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    camera_share = Path(get_package_share_directory('camera_driver'))
    venv_python = Path(os.environ.get('VIRTUAL_ENV', str(Path.home() / 'venv'))) / 'bin/python3'
    python = str(venv_python) if venv_python.is_file() else sys.executable
    conda = os.environ.get('CONDA_EXE') or shutil.which('conda')
    if conda:
        engineer_python = Path(conda).resolve().parents[1] / 'envs/engineer/bin/python'
        if engineer_python.is_file():
            python = str(engineer_python)
    node = Node(package='video_recorder', executable='video_recorder_node', name='video_recorder',
                prefix=[python], additional_env={'PYTHONNOUSERSITE': '1'},
                output='screen', parameters=[LaunchConfiguration('camera_config'), {
                    'camera_config': LaunchConfiguration('camera_config'),
                }])
    return LaunchDescription([
        DeclareLaunchArgument('camera_config', default_value=str(camera_share / 'config/camera.yaml')),
        RegisterEventHandler(OnProcessExit(target_action=node,
            on_exit=[EmitEvent(event=Shutdown(reason='RGB recorder exited'))])),
        node,
    ])
