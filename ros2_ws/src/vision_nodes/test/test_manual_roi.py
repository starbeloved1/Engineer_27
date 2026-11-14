"""选框状态与真实 ROS 消息回验，不打开 GUI、不加载 YOLO、不启动相机。"""
import os
import time
import unittest
from types import SimpleNamespace
from unittest.mock import Mock, patch

import cv2
import numpy as np
import rclpy
from rclpy.executors import SingleThreadedExecutor
from rclpy.node import Node
from rclpy.parameter import Parameter
from sensor_msgs.msg import Image
from std_srvs.srv import Trigger
from vision_msgs.msg import Detection2DArray

from vision_nodes.manual_roi import ManualRoiSelector
from vision_nodes.yolo_detector_node import YoloDetector


def mouse(selector, event, x, y):
    selector.mouse_callback(event, x, y, 0, None)


class SelectionTests(unittest.TestCase):
    def setUp(self):
        self.selector = ManualRoiSelector()
        self.selector.update_image(np.zeros((240, 320, 3), np.uint8), 'color')

    def select(self, a, b):
        mouse(self.selector, cv2.EVENT_LBUTTONDOWN, *a)
        mouse(self.selector, cv2.EVENT_LBUTTONUP, *b)

    def test_selection_persists_until_next_completed_drag(self):
        self.select((20, 30), (120, 130))
        for i in range(10):
            self.selector.update_image(np.full((240, 320, 3), i, np.uint8), 'color')
            self.assertEqual(self.selector.roi, (20, 30, 120, 130))
        mouse(self.selector, cv2.EVENT_LBUTTONDOWN, 200, 180)
        mouse(self.selector, cv2.EVENT_MOUSEMOVE, 50, 80)
        self.assertEqual(self.selector.roi, (20, 30, 120, 130))
        mouse(self.selector, cv2.EVENT_LBUTTONUP, 50, 80)
        self.assertEqual(self.selector.roi, (50, 80, 200, 180))

    def test_click_cancel_and_clipping(self):
        self.select((20, 30), (120, 130))
        self.select((50, 50), (50, 50))
        self.assertEqual(self.selector.roi, (20, 30, 120, 130))
        mouse(self.selector, cv2.EVENT_LBUTTONDOWN, 80, 80)
        self.selector.cancel_drag()
        mouse(self.selector, cv2.EVENT_LBUTTONUP, 200, 200)
        self.assertEqual(self.selector.roi, (20, 30, 120, 130))
        self.select((300, 200), (500, 500))
        self.assertEqual(self.selector.roi, (300, 200, 319, 239))

    def test_closed_window_and_reopen_keep_roi(self):
        self.select((20, 30), (120, 130))
        self.selector.opened = True
        with patch('cv2.getWindowProperty', side_effect=cv2.error('closed')):
            self.selector.poll()
        self.assertTrue(self.selector.hidden)
        self.selector.reopen()
        self.assertFalse(self.selector.hidden)
        self.assertEqual(self.selector.roi, (20, 30, 120, 130))

    def test_resolution_or_frame_change_invalidates_pixel_coordinates(self):
        self.select((20, 30), (120, 130))
        self.assertTrue(self.selector.update_image(np.zeros((480, 640, 3), np.uint8), 'color'))
        self.assertIsNone(self.selector.roi)
        self.select((20, 30), (120, 130))
        self.assertTrue(self.selector.update_image(np.zeros((480, 640, 3), np.uint8), 'other'))
        self.assertIsNone(self.selector.roi)


class DetectorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.old_domain = os.environ.get('ROS_DOMAIN_ID')
        os.environ['ROS_DOMAIN_ID'] = '195'
        rclpy.init(args=['--ros-args', '-p', 'use_manual_roi:=true',
                         '-p', 'model_path:=/does/not/exist.pt',
                         '-p', 'color_topic:=/manual_roi_test/color',
                         '-p', 'detection_topic:=/manual_roi_test/detections',
                         '-p', 'log_every_n:=0'])

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()
        if cls.old_domain is None:
            os.environ.pop('ROS_DOMAIN_ID', None)
        else:
            os.environ['ROS_DOMAIN_ID'] = cls.old_domain

    def setUp(self):
        self.gui_patch = patch.object(ManualRoiSelector, 'poll')
        self.gui_patch.start()
        self.detector = YoloDetector()
        self.peer = Node('manual_roi_test_peer')
        self.executor = SingleThreadedExecutor()
        self.executor.add_node(self.detector)
        self.executor.add_node(self.peer)
        self.messages = []
        self.publisher = self.peer.create_publisher(Image, '/manual_roi_test/color', 10)
        self.subscription = self.peer.create_subscription(
            Detection2DArray, '/manual_roi_test/detections', self.messages.append, 10)
        self.wait(lambda: self.publisher.get_subscription_count() > 0)

    def tearDown(self):
        self.executor.shutdown()
        self.peer.destroy_node()
        self.detector.destroy_node()
        self.gui_patch.stop()

    def wait(self, condition):
        end = time.monotonic() + 3
        while time.monotonic() < end:
            self.executor.spin_once(timeout_sec=0.02)
            if condition():
                return
        self.fail('等待 ROS 图像/框消息超时')

    def frame(self, frame_id='color', width=320, height=240):
        image = Image(width=width, height=height, encoding='bgr8', step=width * 3,
                      data=bytes(width * height * 3))
        image.header.frame_id = frame_id
        image.header.stamp = self.peer.get_clock().now().to_msg()
        self.publisher.publish(image)
        self.wait(lambda: self.messages and self.messages[-1].header.stamp == image.header.stamp)
        return self.messages[-1]

    def assert_box(self, message, expected):
        self.assertEqual(len(message.detections), 1)
        detection = message.detections[0]
        self.assertEqual(detection.header, message.header)
        self.assertEqual(detection.id, 'manual_roi')
        self.assertEqual(detection.results[0].hypothesis.class_id, 'manual_roi')
        box = detection.bbox
        center = box.center.position
        actual = (center.x - box.size_x / 2, center.y - box.size_y / 2,
                  center.x + box.size_x / 2, center.y + box.size_y / 2)
        self.assertEqual(actual, expected)

    def test_fixed_box_fresh_timestamps_reselection_and_mode_switch(self):
        self.assertIsNone(self.detector.model)  # 缺失权重也能启动纯手动模式。
        self.assertFalse(self.frame().detections)
        selector = self.detector.manual_roi
        mouse(selector, cv2.EVENT_LBUTTONDOWN, 20, 30)
        mouse(selector, cv2.EVENT_LBUTTONUP, 120, 130)
        first = self.frame()
        self.assert_box(first, (20, 30, 120, 130))
        second = self.frame()
        self.assert_box(second, (20, 30, 120, 130))
        self.assertNotEqual(first.header.stamp, second.header.stamp)
        mouse(selector, cv2.EVENT_LBUTTONDOWN, 100, 80)
        mouse(selector, cv2.EVENT_MOUSEMOVE, 210, 190)
        self.assert_box(self.frame(), (20, 30, 120, 130))
        mouse(selector, cv2.EVENT_LBUTTONUP, 210, 190)
        self.assert_box(self.frame(), (100, 80, 210, 190))
        selector.hidden = True
        self.assert_box(self.frame(), (100, 80, 210, 190))
        client = self.peer.create_client(Trigger, '/yolo_detector/select_roi')
        self.assertTrue(client.wait_for_service(timeout_sec=1))
        future = client.call_async(Trigger.Request())
        self.wait(future.done)
        self.assertTrue(future.result().success)
        self.assertFalse(selector.hidden)
        self.assertEqual(selector.roi, (100, 80, 210, 190))
        model = SimpleNamespace(predict=Mock(return_value=[SimpleNamespace(boxes=None)]))
        self.detector.model = model
        self.detector.set_parameters([Parameter('use_manual_roi', value=False)])
        self.assertFalse(self.frame().detections)
        model.predict.assert_called_once()
        self.detector.set_parameters([Parameter('use_manual_roi', value=True)])
        self.assert_box(self.frame(), (100, 80, 210, 190))
        model.predict.assert_called_once()  # 手动模式不运行 YOLO。

    def test_image_geometry_change_clears_manual_output(self):
        self.frame()
        mouse(self.detector.manual_roi, cv2.EVENT_LBUTTONDOWN, 20, 30)
        mouse(self.detector.manual_roi, cv2.EVENT_LBUTTONUP, 120, 130)
        self.assert_box(self.frame(), (20, 30, 120, 130))
        self.assertFalse(self.frame(width=640, height=480).detections)
        self.assertIsNone(self.detector.manual_roi.roi)


if __name__ == '__main__':
    unittest.main()
