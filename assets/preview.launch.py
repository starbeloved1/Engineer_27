"""Preview the exported arm in ROS 2; mesh paths are resolved in memory."""

from pathlib import Path
import xml.etree.ElementTree as ET

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, OpaqueFunction
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def load_description(urdf_path):
    path = Path(urdf_path).expanduser().resolve()
    root = ET.parse(path).getroot()
    for mesh in root.iter("mesh"):
        filename = mesh.get("filename", "")
        if filename and "://" not in filename:
            local_path = Path(filename)
            if not local_path.is_absolute():
                local_path = path.parent / local_path
            if not local_path.is_file():
                raise FileNotFoundError(f"Mesh not found: {local_path}")
            mesh.set("filename", local_path.resolve().as_uri())
    return ET.tostring(root, encoding="unicode")


def create_nodes(context):
    description = load_description(LaunchConfiguration("urdf").perform(context))
    parameters = [{
        "robot_description": ParameterValue(description, value_type=str),
        "use_sim_time": False,
    }]
    return [
        Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            name="arm_preview_state_publisher",
            parameters=parameters,
            output="screen",
        ),
        GroupAction(
            condition=IfCondition(LaunchConfiguration("joint_states")),
            actions=[
                Node(
                    package="joint_state_publisher_gui",
                    executable="joint_state_publisher_gui",
                    parameters=parameters,
                    condition=IfCondition(LaunchConfiguration("gui")),
                    output="screen",
                ),
                Node(
                    package="joint_state_publisher",
                    executable="joint_state_publisher",
                    parameters=parameters,
                    condition=UnlessCondition(LaunchConfiguration("gui")),
                    output="screen",
                ),
            ],
        ),
        Node(
            package="rviz2",
            executable="rviz2",
            arguments=["-d", str(Path(LaunchConfiguration("rviz_config").perform(context)).expanduser().resolve())],
            condition=IfCondition(LaunchConfiguration("rviz")),
            output="screen",
        ),
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            "urdf", default_value=str(Path(__file__).with_name("arm.urdf"))
        ),
        DeclareLaunchArgument("gui", default_value="true"),
        DeclareLaunchArgument("rviz", default_value="true"),
        DeclareLaunchArgument("rviz_config", default_value=str(Path(__file__).with_name("preview.rviz"))),
        DeclareLaunchArgument("joint_states", default_value="true"),
        OpaqueFunction(function=create_nodes),
    ])
