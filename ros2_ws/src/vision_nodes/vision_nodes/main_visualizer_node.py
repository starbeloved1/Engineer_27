"""统一发布视觉调试图，显示由 Foxglove 完成"""
import math
import sys
import time

import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Image


class MainVisualizer(Node):
    def __init__(self):
        super().__init__("main_visualizer")

        # 参数归属本节点，默认值与包内 vision_params.yaml 保持一致
        self.color_topic = self.declare_parameter(
            "color_topic", "/camera/camera/color/image_raw").value
        self.visualization_topic = self.declare_parameter(
            "visualization_topic", "/vision/visualization/image").value
        self.visualization_rate_hz = float(self.declare_parameter(
            "visualization_rate_hz", 0.0).value)
        reliability = self.declare_parameter("input_reliability", "reliable").value
        if not self.color_topic or not self.visualization_topic:
            raise ValueError("图像输入和可视化输出话题不能为空")
        if (self.resolve_topic_name(self.color_topic)
                == self.resolve_topic_name(self.visualization_topic)):
            raise ValueError("图像输入和可视化输出话题不能相同")
        if not math.isfinite(self.visualization_rate_hz) or self.visualization_rate_hz < 0:
            raise ValueError("visualization_rate_hz 必须是有限非负数")
        if reliability not in ("reliable", "best_effort"):
            raise ValueError("input_reliability 必须是 reliable 或 best_effort")

        # 只保留最新一帧，本机图像使用可靠传输避免大消息丢帧
        input_qos = QoSProfile(depth=1, reliability=(ReliabilityPolicy.RELIABLE
                              if reliability == "reliable"
                              else ReliabilityPolicy.BEST_EFFORT))
        output_qos = QoSProfile(depth=1, reliability=ReliabilityPolicy.RELIABLE)
        self.image_pub = self.create_publisher(Image, self.visualization_topic, output_qos)
        self.image_sub = self.create_subscription(
            Image, self.color_topic, self.image_callback, input_qos)
        self.visualization_period = (1.0 / self.visualization_rate_hz
                                     if self.visualization_rate_hz > 0 else 0.0)
        self.last_visualization_time = None
        self.get_logger().info(
            f"Main visualizer: {self.color_topic} -> {self.visualization_topic}")

    def image_callback(self, message: Image):
        # 参考 Hero 的按需发布方式，无显示端订阅时不生成调试输出
        if self.image_pub.get_subscription_count() == 0:
            return
        now = time.monotonic()
        if (self.last_visualization_time is not None
                and now - self.last_visualization_time < self.visualization_period):
            return

        # 当前尚无检测结果，原样发布原图，保留采集时间、坐标系和编码
        # 接入检测后，在本节点用对应原图绘制调试信息，不改动算法输入
        self.image_pub.publish(message)
        self.last_visualization_time = now


def main(args=None):
    rclpy.init(args=args)
    node = None
    try:
        node = MainVisualizer()
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    except Exception as error:
        print(f"main_visualizer failed: {error}", file=sys.stderr)
        return 1
    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
    return 0


if __name__ == "__main__":
    sys.exit(main())
