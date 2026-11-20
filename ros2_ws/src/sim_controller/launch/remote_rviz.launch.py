"""电脑端完整 MTC 显示；仅启动 RViz，不启动硬件、规划器或状态发布器。"""
from launch import LaunchDescription
from sim_controller.launch_support import rviz_node


def generate_launch_description():
    return LaunchDescription([rviz_node(mtc_details=True)])
