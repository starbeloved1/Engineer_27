"""仅附加机械臂显示，不启动模拟硬件、控制器或规划服务。"""
from launch import LaunchDescription
from sim_controller.launch_support import rviz_node


def generate_launch_description():
    return LaunchDescription([rviz_node(native=True)])
