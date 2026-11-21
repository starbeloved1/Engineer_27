"""独立预览 URDF；相对网格路径在内存中解析，不修改模型文件。"""

from pathlib import Path
import math
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


def add_d435(description):
    """只补充预览几何和标称传感器坐标系，不改安装外参或源 URDF。"""
    root = ET.fromstring(description)
    if root.find("link[@name='camera_link']") is None:
        raise ValueError("D435 预览需要模型中已有 camera_link 安装坐标系")
    for element in root.findall("link/visual"):
        material = element.find("material")
        if material is not None:
            element.remove(material)
        material = ET.SubElement(element, "material", name="preview_robot_grey")
        ET.SubElement(material, "color", rgba="0.58 0.63 0.68 1")

    def fixed_link(name, parent, xyz="0 0 0", rpy="0 0 0"):
        if root.find(f"link[@name='{name}']") is not None:
            raise ValueError(f"预览坐标系重名: {name}")
        link = ET.SubElement(root, "link", name=name)
        joint = ET.SubElement(root, "joint", name=name + "_joint", type="fixed")
        ET.SubElement(joint, "parent", link=parent)
        ET.SubElement(joint, "child", link=name)
        ET.SubElement(joint, "origin", xyz=xyz, rpy=rpy)
        return link

    def visual(link, name, kind, attributes, xyz="0 0 0", rpy="0 0 0", color="0.7 0.75 0.8 1"):
        element = ET.SubElement(link, "visual", name=name)
        ET.SubElement(element, "origin", xyz=xyz, rpy=rpy)
        geometry = ET.SubElement(element, "geometry")
        ET.SubElement(geometry, kind, **attributes)
        material = ET.SubElement(element, "material", name=name + "_material")
        ET.SubElement(material, "color", rgba=color)

    # ROS 相机系为 X 向前、Y 向左、Z 向上：长 90 mm 沿 Y，深/高各 25 mm。
    # 外壳中心相对 camera_link 的 Y=-17.5 mm，取自 RealSense 标称示意；非实测标定。
    body = fixed_link("d435_body_frame", "camera_link", xyz="0 -0.0175 0")
    visual(body, "d435_case", "box", {"size": "0.025 0.090 0.025"})
    visual(body, "d435_front", "box", {"size": "0.001 0.086 0.021"},
           xyz="0.0121 0 0", color="0.035 0.045 0.060 1")
    for index, y in enumerate((-0.032, -0.011, 0.011, 0.032)):
        visual(body, f"d435_lens_{index}", "cylinder", {"radius": "0.003", "length": "0.001"},
               xyz=f"0.0128 {y} 0", rpy=f"0 {math.pi / 2} 0", color="0.10 0.40 0.75 1")

    # 与驱动一致的轴约定；彩色相对深度的位置只用标称值，不代表这台 D435 的出厂外参。
    optical_rpy = f"{-math.pi / 2} 0 {-math.pi / 2}"
    fixed_link("camera_depth_frame", "camera_link")
    fixed_link("camera_depth_optical_frame", "camera_depth_frame", rpy=optical_rpy)
    fixed_link("camera_color_frame", "camera_link", xyz="0 0.015 0")
    fixed_link("camera_color_optical_frame", "camera_color_frame", rpy=optical_rpy)
    return ET.tostring(root, encoding="unicode")


def create_nodes(context):
    description = load_description(LaunchConfiguration("urdf").perform(context))
    if IfCondition(LaunchConfiguration("camera")).evaluate(context):
        description = add_d435(description)
    root = ET.fromstring(description)
    children = {joint.find("child").get("link") for joint in root.findall("joint")}
    fixed_frame = next(link.get("name") for link in root.findall("link")
                       if link.get("name") not in children)
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
            arguments=["-d", str(Path(LaunchConfiguration("rviz_config").perform(context)).expanduser().resolve()),
                       "-f", fixed_frame],
            condition=IfCondition(LaunchConfiguration("rviz")),
            output="screen",
        ),
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            "urdf", default_value=str(Path(__file__).resolve().parents[1] / "assets/engineer/arm.urdf")
        ),
        DeclareLaunchArgument("gui", default_value="true"),
        DeclareLaunchArgument("rviz", default_value="true"),
        DeclareLaunchArgument("rviz_config", default_value=str(Path(__file__).with_name("preview.rviz"))),
        DeclareLaunchArgument("joint_states", default_value="true"),
        DeclareLaunchArgument("camera", default_value="false"),
        OpaqueFunction(function=create_nodes),
    ])
