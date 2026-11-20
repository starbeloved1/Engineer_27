"""投影几何、PointCloud2 布局与过期/缺失数据回退。"""
from array import array
import struct
import time

import cv2
import unittest

import numpy as np
import rclpy
from geometry_msgs.msg import TransformStamped
from rclpy.node import Node
from sensor_msgs.msg import CameraInfo, CompressedImage, PointCloud2, PointField
from vision_msgs.msg import Detection2DArray
from yolo_detector.yolo_detector_node import YoloDetector
from std_msgs.msg import Header

from yolo_detector.cloud_overlay import CloudOverlay, cloud_xyz, project_pixels


def make_cloud():
    cloud = PointCloud2()
    cloud.header = Header(frame_id='model_frame')
    cloud.header.stamp.sec = 10
    cloud.height, cloud.width = 2, 2
    cloud.point_step, cloud.row_step = 16, 40  # 包含点/行填充
    cloud.is_bigendian = True
    cloud.fields = [PointField(name=name, offset=i*4, datatype=PointField.FLOAT32, count=1)
                    for i, name in enumerate(('x', 'y', 'z'))]
    data = bytearray(80)
    for index, xyz in enumerate(((0, 0, 1), (0, 0, -1), (float('nan'), 0, 1), (100, 0, 1))):
        struct.pack_into('>fff', data, index//2*40 + index%2*16, *xyz)
    cloud.data = array('B', data)
    return cloud


def make_camera():
    camera = CameraInfo()
    camera.header.frame_id = 'color_optical'
    camera.width, camera.height = 100, 80
    camera.k = [100., 0., 50., 0., 100., 40., 0., 0., 1.]
    camera.distortion_model = 'plumb_bob'
    camera.d = [0.] * 5
    return camera


class ProjectionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init(args=['--ros-args', '-p', 'use_manual_roi:=true'])

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def test_layout_and_projection(self):
        cloud, camera = make_cloud(), make_camera()
        xyz = cloud_xyz(cloud, 3000)
        np.testing.assert_array_equal(xyz[0], [0, 0, 1])
        self.assertEqual(len(cloud_xyz(cloud, 2)), 2)
        np.testing.assert_array_equal(project_pixels(xyz, camera, 100, 80), [[50, 40]])
        camera.d = [2.0, 0., 0., 0., 0.]
        np.testing.assert_array_equal(project_pixels(np.array([[0.5, 0., 2.]]), camera, 100, 80), [[78, 40]])

    def test_compressed_box_and_cloud_delivery(self):
        node, receiver = YoloDetector(), Node('overlay_jpeg_test')
        received = []
        sub = receiver.create_subscription(CompressedImage,
            '/vision/visualization/image/compressed', received.append, 1)
        try:
            deadline = time.monotonic() + 5
            while node.compressed_visualization_pub.get_subscription_count() == 0 and time.monotonic() < deadline:
                rclpy.spin_once(receiver, timeout_sec=0.1)
            self.assertGreater(node.compressed_visualization_pub.get_subscription_count(), 0)
            cloud, camera = make_cloud(), make_camera()
            cloud.header.frame_id = camera.header.frame_id
            node.cloud_overlay.receive_camera(camera)
            node.cloud_overlay.clouds.append(cloud)
            detections = Detection2DArray(header=cloud.header)
            node.append_detection(detections, cloud.header, (10, 10, 90, 70), 'energy', 0.95)
            node.publish_visualization(np.zeros((80, 100, 3), dtype=np.uint8), detections)
            deadline = time.monotonic() + 5
            while not received and time.monotonic() < deadline:
                rclpy.spin_once(receiver, timeout_sec=0.1)
            self.assertTrue(received)
            msg = received[0]
            image = cv2.imdecode(np.frombuffer(msg.data, dtype=np.uint8), cv2.IMREAD_COLOR)
            self.assertGreater(int(image[40, 50, 2]) - int(image[40, 50, 1]), 10)
            self.assertGreater(int(image[60, 10, 1]), 100)
            self.assertEqual(msg.header, cloud.header)
        finally:
            node.destroy_node()
            receiver.destroy_node()

    def test_tf_stale_and_empty_fallback(self):
        node = Node('test_overlay')
        try:
            overlay = CloudOverlay(node)
            camera, cloud = make_camera(), make_cloud()
            overlay.receive_camera(camera)
            overlay.clouds.append(cloud)
            header = Header(frame_id='color_optical', stamp=cloud.header.stamp)
            image = np.zeros((80, 100, 3), dtype=np.uint8)
            np.testing.assert_array_equal(overlay.draw(image, header), image)  # 缺 TF
            tf = TransformStamped()
            tf.header.frame_id, tf.child_frame_id = 'color_optical', 'model_frame'
            tf.transform.rotation.w = 1.
            tf.transform.translation.x = 0.1
            overlay.tf_buffer.set_transform_static(tf, 'test')
            output = overlay.draw(image, header)
            np.testing.assert_array_equal(output[40, 60], [0, 80, 255])
            self.assertFalse(image.any())  # 不修改检测输入
            header = Header(frame_id="color_optical")
            header.stamp.sec = 12  # 超过默认 1.5 秒门限
            np.testing.assert_array_equal(overlay.draw(image, header), image)  # 过期
            empty = PointCloud2(header=header)
            overlay.clouds.append(empty)
            np.testing.assert_array_equal(overlay.draw(image, header), image)
        finally:
            node.destroy_node()


if __name__ == '__main__':
    unittest.main()
