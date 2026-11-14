"""彩色图 → YOLO 或固定手动 ROI，只发布二维框，绘图仍交给 main_visualizer。"""
import math
from pathlib import Path
import sys
import time

from cv_bridge import CvBridge, CvBridgeError
import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Image
from std_srvs.srv import Trigger
from vision_msgs.msg import Detection2D, Detection2DArray, ObjectHypothesisWithPose

from .manual_roi import ManualRoiSelector


class YoloDetector(Node):
    def __init__(self):
        super().__init__("yolo_detector")

        self.color_topic = self.declare_parameter(
            "color_topic", "/camera/camera/color/image_raw").value
        self.detection_topic = self.declare_parameter(
            "detection_topic", "/vision/yolo/detections").value
        self.use_manual_roi = self.declare_parameter("use_manual_roi", False).value
        self.manual_roi = ManualRoiSelector()
        model_path = self.declare_parameter(
            "model_path", "assets/best.pt").value
        self.conf_thres = float(self.declare_parameter("conf_thres", 0.5).value)
        self.iou_thres = float(self.declare_parameter("iou_thres", 0.45).value)
        self.device = self.declare_parameter("device", "cpu").value
        self.imgsz = self.declare_parameter("imgsz", 640).value
        self.log_every_n = self.declare_parameter("log_every_n", 30).value
        reliability = self.declare_parameter("input_reliability", "reliable").value
        if not self.color_topic or not self.detection_topic:
            raise ValueError("图像和检测话题不能为空")
        if not all(math.isfinite(v) and 0 <= v <= 1
                   for v in (self.conf_thres, self.iou_thres)):
            raise ValueError("conf_thres 和 iou_thres 必须在 [0, 1] 范围内")
        if self.imgsz <= 0 or self.imgsz % 32 or self.log_every_n < 0:
            raise ValueError("imgsz 必须是 32 的正整数倍，log_every_n 不能为负数")
        if reliability not in ("reliable", "best_effort"):
            raise ValueError("input_reliability 必须是 reliable 或 best_effort")
        self.model_path = Path(model_path).expanduser().resolve()
        self.model = None
        # 手动模式不加载权重或调用推理；首次恢复 YOLO 时才加载。
        if not self.use_manual_roi:
            self.load_model()
        self.bridge = CvBridge()
        self.frame_count = 0
        input_qos = QoSProfile(depth=1, reliability=(ReliabilityPolicy.RELIABLE
                              if reliability == "reliable"
                              else ReliabilityPolicy.BEST_EFFORT))
        result_qos = QoSProfile(depth=10, reliability=ReliabilityPolicy.RELIABLE)
        self.detection_pub = self.create_publisher(
            Detection2DArray, self.detection_topic, result_qos)
        self.image_sub = self.create_subscription(
            Image, self.color_topic, self.image_callback, input_qos)
        self.roi_window_timer = self.create_timer(1.0 / 30.0, self.poll_manual_roi)
        self.roi_service = self.create_service(Trigger, '~/select_roi', self.select_roi)
        self.get_logger().info(
            f"Detector ready: manual_roi={self.use_manual_roi}, model={self.model_path}, "
            f"device={self.device}; {self.color_topic} -> {self.detection_topic}")

    def load_model(self):
        if self.model is None:
            if not self.model_path.is_file():
                raise FileNotFoundError(f"YOLO 权重不存在: {self.model_path}")
            # 加载失败直接退出，不以空检测伪装成正常推理。
            from ultralytics import YOLO
            self.model = YOLO(str(self.model_path), task="detect")

    def update_manual_mode(self):
        enabled = self.get_parameter('use_manual_roi').value
        if enabled != self.use_manual_roi:
            self.use_manual_roi = enabled
            if enabled:
                self.manual_roi.reopen()
            else:
                self.manual_roi.close()
            self.get_logger().info(f"检测模式切换为 {'手动 ROI' if enabled else 'YOLO'}")

    def poll_manual_roi(self):
        self.update_manual_mode()
        if self.use_manual_roi:
            previous = self.manual_roi.roi
            self.manual_roi.poll()
            if self.manual_roi.roi != previous:
                self.get_logger().info(f"固定手动 ROI: {self.manual_roi.roi}，重新拖框后才替换")

    def select_roi(self, request, response):
        self.update_manual_mode()
        response.success = self.use_manual_roi
        if self.use_manual_roi:
            self.manual_roi.reopen()
            response.message = '已请求显示选框窗口；拖动完成前继续沿用旧 ROI'
        else:
            response.message = '请先设置 /yolo_detector 的 use_manual_roi=true'
        return response

    @staticmethod
    def append_detection(detections, header, xyxy, class_id, confidence, manual=False):
        x1, y1, x2, y2 = map(float, xyxy)
        detection = Detection2D()
        detection.header = header
        detection.bbox.center.position.x = (x1 + x2) / 2.0
        detection.bbox.center.position.y = (y1 + y2) / 2.0
        detection.bbox.size_x = x2 - x1
        detection.bbox.size_y = y2 - y1
        if manual:
            detection.id = 'manual_roi'
        hypothesis = ObjectHypothesisWithPose()
        hypothesis.hypothesis.class_id = class_id
        hypothesis.hypothesis.score = float(confidence)
        detection.results.append(hypothesis)
        detections.detections.append(detection)

    def image_callback(self, message: Image):
        try:
            image = self.bridge.imgmsg_to_cv2(message, desired_encoding="bgr8")
        except CvBridgeError as error:
            self.get_logger().error(f"图像转换失败: {error}", throttle_duration_sec=5.0)
            return

        start = time.perf_counter()
        self.update_manual_mode()
        detections = Detection2DArray()
        detections.header = message.header
        if self.use_manual_roi:
            if self.manual_roi.update_image(image, message.header.frame_id):
                self.get_logger().warning('彩色图尺寸或 frame_id 改变，手动 ROI 已失效，请重新拖框')
            if self.manual_roi.roi is not None:
                # 每帧重建消息与采集时间戳，框坐标保持不变，不伪装成目标跟踪。
                self.append_detection(detections, message.header, self.manual_roi.roi,
                                      'manual_roi', 1.0, manual=True)
        else:
            self.load_model()
            result = self.model.predict(
                source=image, conf=self.conf_thres, iou=self.iou_thres,
                device=self.device, imgsz=self.imgsz, verbose=False, save=False, show=False)[0]
            if result.boxes is not None:
                boxes = result.boxes.cpu()
                for xyxy, confidence, class_id in zip(
                        boxes.xyxy.numpy(), boxes.conf.numpy(), boxes.cls.numpy()):
                    self.append_detection(detections, message.header, xyxy,
                                          str(int(class_id)), confidence)

        # 空数组明确表示这一帧没有检测到目标，时间戳仍是原图采集时刻
        self.detection_pub.publish(detections)
        self.frame_count += 1
        if self.frame_count == 1 or (self.log_every_n and self.frame_count % self.log_every_n == 0):
            elapsed = (time.perf_counter() - start) * 1000
            self.get_logger().info(
                f"{'Manual ROI' if self.use_manual_roi else 'YOLO'}: {elapsed:.1f} ms, "
                f"{len(detections.detections)} detections, "
                f"input={message.width}x{message.height}")

    def destroy_node(self):
        self.manual_roi.close()
        return super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = None
    try:
        node = YoloDetector()
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    except Exception as error:
        print(f"yolo_detector failed: {error}", file=sys.stderr)
        return 1
    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
    return 0


if __name__ == "__main__":
    sys.exit(main())
