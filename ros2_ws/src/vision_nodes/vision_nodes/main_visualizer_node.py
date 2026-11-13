"""统一发布视觉调试图，显示由 Foxglove 完成"""
import math
import sys
import time

from cv_bridge import CvBridge, CvBridgeError
from message_filters import Subscriber, TimeSynchronizer
import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Image
from vision_msgs.msg import Detection2DArray

from .detection_visualization import draw_detections


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
        self.detection_topic = self.declare_parameter(
            "detection_topic", "/vision/yolo/detections").value
        self.draw_boxes = self.declare_parameter("draw_detections", True).value
        queue_size = self.declare_parameter("sync_queue_size", 30).value
        if not self.detection_topic or queue_size <= 0:
            raise ValueError("detection_topic 不能为空，sync_queue_size 必须为正整数")
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
        self.bridge = CvBridge()
        if self.draw_boxes:
            # 推理有延迟，缓存原图并按采集时间戳精确配对，禁止借用相邻帧的框
            self.image_sub = Subscriber(self, Image, self.color_topic, qos_profile=input_qos)
            self.detection_sub = Subscriber(
                self, Detection2DArray, self.detection_topic,
                qos_profile=QoSProfile(depth=10, reliability=ReliabilityPolicy.RELIABLE))
            self.synchronizer = TimeSynchronizer(
                [self.image_sub, self.detection_sub], queue_size=queue_size)
            self.synchronizer.registerCallback(self.image_callback)
        else:
            self.image_sub = self.create_subscription(
                Image, self.color_topic, self.image_callback, input_qos)
        self.visualization_period = (1.0 / self.visualization_rate_hz
                                     if self.visualization_rate_hz > 0 else 0.0)
        self.last_visualization_time = None
        self.get_logger().info(
            f"Main visualizer: {self.color_topic} -> {self.visualization_topic}, "
            f"draw_detections={self.draw_boxes}")

    def image_callback(self, message: Image, detections=None):
        # 参考 Hero 的按需发布方式，无显示端订阅时不生成调试输出
        if self.image_pub.get_subscription_count() == 0:
            return
        now = time.monotonic()
        if (self.last_visualization_time is not None
                and now - self.last_visualization_time < self.visualization_period):
            return

        if self.draw_boxes:
            if detections is None or message.header.frame_id != detections.header.frame_id:
                self.get_logger().warning("检测结果与原图坐标系不一致，忽略本帧",
                                          throttle_duration_sec=5.0)
                return
            boxes = []
            for detection in detections.detections:
                if not detection.results:
                    continue
                hypothesis = max(detection.results, key=lambda item: item.hypothesis.score).hypothesis
                center = detection.bbox.center.position
                half_width, half_height = detection.bbox.size_x / 2, detection.bbox.size_y / 2
                boxes.append((center.x - half_width, center.y - half_height,
                              center.x + half_width, center.y + half_height,
                              hypothesis.class_id, hypothesis.score))
            try:
                image = self.bridge.imgmsg_to_cv2(message, desired_encoding="bgr8")
                overlay = draw_detections(image, boxes)
                output = self.bridge.cv2_to_imgmsg(overlay, encoding="bgr8")
                output.header = message.header
            except (CvBridgeError, ValueError) as error:
                self.get_logger().error(f"调试图绘制失败: {error}", throttle_duration_sec=5.0)
                return
        else:
            output = message
        self.image_pub.publish(output)
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
