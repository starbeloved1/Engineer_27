"""不占用相机：几何回验、真实 ROS 配对、TF 与失效清空。

在 Engineer_27 根目录 source ros2_ws/install/setup.bash 后，用 engineer 的
Python 运行本文件。系统 colcon 若没有 Open3D，会明确跳过此模块。
"""
import importlib.util
from pathlib import Path
import time
import unittest
from types import SimpleNamespace

if importlib.util.find_spec('open3d') is None:
    raise unittest.SkipTest('ICP 测试需要 conda engineer 中的 Open3D')

import numpy as np
import open3d as o3d
import rclpy
from geometry_msgs.msg import PoseArray, TransformStamped
from rclpy.executors import SingleThreadedExecutor
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, QoSProfile
from rclpy.time import Time
from scipy.spatial.transform import Rotation
from sensor_msgs.msg import CameraInfo, PointCloud2
from std_msgs.msg import Header
from tf2_ros import StaticTransformBroadcaster
from vision_msgs.msg import Detection2D, Detection2DArray

from vision_nodes.icp_registration_node import IcpRegistration
from vision_nodes.pointcloud_geometry import (
    accepted_registration, filtered_cloud, load_model, register_model, roi_mask,
)
from vision_nodes.pointcloud_messages import make_cloud, read_cloud


ROOT = Path(__file__).resolve().parents[4]
MODEL = ROOT / 'assets/energy.stl'


class GeometryTests(unittest.TestCase):
    def test_zero_rmse_limit_disables_only_residual_gate(self):
        result = SimpleNamespace(transformation=np.eye(4), fitness=.8,
                                 inlier_rmse=.05, correspondence_set=range(100))
        self.assertFalse(accepted_registration(result, .3, .03, 80))
        self.assertTrue(accepted_registration(result, .3, 0., 80))
        result.inlier_rmse = .03
        self.assertTrue(accepted_registration(result, .3, .03, 80))
        for value in (np.nan, np.inf):
            result.inlier_rmse = value
            self.assertFalse(accepted_registration(result, .3, 0., 80))
        result.inlier_rmse = .05
        result.fitness = .1
        self.assertFalse(accepted_registration(result, .3, 0., 80))
        result.fitness = .8
        result.correspondence_set = range(20)
        self.assertFalse(accepted_registration(result, .3, 0., 80))
        result.correspondence_set = range(100)
        result.transformation[0, 0] = np.nan
        self.assertFalse(accepted_registration(result, .3, 0., 80))

    def test_projection_applies_extrinsic_and_rejects_invalid_depth(self):
        xyz = np.array([[-0.05, 0, 1], [0.1, 0, 1], [0, 0, -1],
                        [0, 0, 3], [np.nan, 0, 1], [0, 0, 0]])
        k = np.array([[200., 0, 100], [0, 200, 100], [0, 0, 1]])
        transform = np.eye(4)
        transform[0, 3] = 0.1
        mask = roi_mask(xyz, k, np.zeros(5), transform, (110, 100, 8, 8, 0),
                        (200, 200), 0.2, 2., 0.)
        np.testing.assert_array_equal(mask, [True, False, False, False, False, False])
        self.assertFalse(roi_mask(xyz, k, np.zeros(5), np.eye(4),
                                 (110, 100, 8, 8, 0), (200, 200), .2, 2., 0.).any())

    def test_rgb_roundtrip_and_big_endian_row_padding(self):
        xyz = np.array([[0.1, 0.2, 0.3], [1, 2, 3], [4, 5, 6], [7, 8, 9]])
        rgb = np.array([[1, 2, 255], [200, 100, 0], [5, 6, 7], [8, 9, 10]], dtype=np.uint8)
        message = make_cloud(Header(frame_id='depth'), xyz, rgb)
        out_xyz, out_rgb = read_cloud(message)
        np.testing.assert_allclose(out_xyz, xyz, atol=1e-7)
        np.testing.assert_array_equal(out_rgb, rgb)
        # 2 行，每行 2 个点 + 8 字节填充；大端转换不应修改输入缓冲区。
        raw = np.frombuffer(bytes(message.data), dtype='<u4').reshape(2, 8)
        padded = b''.join(row.astype('>u4').tobytes() + b'padding!' for row in raw)
        message.width, message.height, message.row_step = 2, 2, 40
        message.is_bigendian = True
        message.data = padded
        out_xyz, out_rgb = read_cloud(message)
        np.testing.assert_allclose(out_xyz, xyz, atol=1e-7)
        np.testing.assert_array_equal(out_rgb, rgb)
        self.assertEqual(bytes(message.data), padded)
        self.assertEqual(read_cloud(make_cloud(Header(), np.empty((0, 3))))[0].shape, (0, 3))

    def test_model_scale_origin_and_rigid_registration(self):
        model = load_model(MODEL, .001, 2000, .005)
        points = np.asarray(model.points)
        self.assertEqual(len(points), 2000)
        np.testing.assert_allclose(np.ptp(points, axis=0), [.095, .095, .150], atol=.003)
        self.assertGreater(points[:, 2].mean(), .05)  # 原点未被移到质心
        # 0.005 的体素在源模型 mm 单位中应用。误当成 m 会合并大量采样点。
        coarse_model = load_model(MODEL, .001, 2000, 10.)
        self.assertLess(len(coarse_model.points), 1000)
        np.testing.assert_allclose(np.ptp(np.asarray(coarse_model.points), axis=0),
                                   [.095, .095, .150], atol=.012)
        rng = np.random.default_rng(27)
        xyz = rng.uniform(-1, 1, (1000, 3)) * [.07, .04, .1]
        source = filtered_cloud(xyz, None, 0., 0, 1.5)
        rotation = Rotation.from_euler('xyz', [4, -3, 2], degrees=True)
        translation = np.array([.1, -.05, .7])
        observed = rotation.apply(xyz) + translation + rng.normal(0, .0001, xyz.shape)
        target = filtered_cloud(observed, None, 0., 0, 1.5)
        result = register_model(source, target, [0, 0, 0], .03, 100)
        self.assertTrue(accepted_registration(result, .9, .001, 80))
        np.testing.assert_allclose(result.transformation[:3, 3], translation, atol=.001)
        self.assertLess((Rotation.from_matrix(result.transformation[:3, :3]).inv()
                         * rotation).magnitude(), .01)
        # 无对应点时 Open3D 的 RMSE 也可能为 0，不能据此误判成功。
        bad = o3d.pipelines.registration.RegistrationResult()
        self.assertFalse(accepted_registration(bad, .3, .015, 80))


class ParameterTests(unittest.TestCase):
    def test_zero_rmse_parameter_starts_node(self):
        rclpy.init(args=['--ros-args', '-p', f'model_path:={MODEL}',
                         '-p', 'max_inlier_rmse:=0'])
        node = None
        try:
            node = IcpRegistration()
            self.assertEqual(node.params['max_inlier_rmse'], 0.)
        finally:
            if node is not None:
                node.destroy_node()
            rclpy.shutdown()


class PipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init(args=['--ros-args', '-p', f'model_path:={MODEL}',
                         '-p', 'max_data_age:=0.6', '-p', 'log_every_n:=0'])

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.registration = IcpRegistration()
        self.peer = Node('icp_test_peer')
        self.executor = SingleThreadedExecutor()
        self.executor.add_node(self.registration)
        self.executor.add_node(self.peer)
        self.messages = {name: [] for name in ('roi', 'target', 'aligned', 'poses', 'model')}
        self.subs = []
        for name, topic, kind in (
                ('roi', '/vision/icp/roi_cloud', PointCloud2),
                ('target', '/vision/icp/target_cloud', PointCloud2),
                ('aligned', '/vision/icp/aligned_cloud', PointCloud2),
                ('poses', '/vision/icp/object_poses', PoseArray),
                ('model', '/vision/model_cloud', PointCloud2)):
            qos = QoSProfile(depth=10, durability=(DurabilityPolicy.TRANSIENT_LOCAL
                              if name == 'model' else DurabilityPolicy.VOLATILE))
            self.subs.append(self.peer.create_subscription(
                kind, topic, self.messages[name].append, qos))
        self.cloud_pub = self.peer.create_publisher(PointCloud2, '/camera/camera/depth/color/points', 10)
        self.det_pub = self.peer.create_publisher(Detection2DArray, '/vision/yolo/detections', 10)
        self.info_pub = self.peer.create_publisher(CameraInfo, '/camera/camera/color/camera_info', 10)
        self.broadcaster = StaticTransformBroadcaster(self.peer)
        transform = TransformStamped()
        transform.header.frame_id = 'color'
        transform.child_frame_id = 'depth'
        transform.transform.translation.x = .1
        transform.transform.rotation.w = 1.
        self.broadcaster.sendTransform(transform)
        self.spin_until(lambda: (self.cloud_pub.get_subscription_count() > 0
                                and self.det_pub.get_subscription_count() > 0
                                and self.info_pub.get_subscription_count() > 0
                                and self.registration.tf_buffer.can_transform(
                                    'color', 'depth', Time()))
                        and len(self.messages['model']) > 0)
        self.info = CameraInfo()
        self.info.header.frame_id = 'color'
        self.info.width, self.info.height = 640, 480
        self.info.k = [300., 0., 320., 0., 300., 240., 0., 0., 1.]
        self.info.d = [0.] * 5
        self.info.distortion_model = 'plumb_bob'
        self.info_pub.publish(self.info)
        self.spin_until(lambda: self.registration.camera_info is not None)

    def tearDown(self):
        self.executor.remove_node(self.registration)
        self.executor.remove_node(self.peer)
        self.registration.destroy_node()
        self.peer.destroy_node()
        self.executor.shutdown()

    def spin_until(self, predicate, timeout=3.):
        deadline = time.monotonic() + timeout
        while not predicate() and time.monotonic() < deadline:
            self.executor.spin_once(timeout_sec=.01)
        self.assertTrue(predicate(), '等待 ROS 合成链路输出超时')

    def spin_for(self, seconds):
        deadline = time.monotonic() + seconds
        while time.monotonic() < deadline:
            self.executor.spin_once(timeout_sec=.01)

    def send(self, empty=False, frame='depth', offset=0., age=0.):
        stamp = Time(nanoseconds=self.peer.get_clock().now().nanoseconds - int(age * 1e9))
        source = self.registration.source_xyz
        xyz = source + [-.1, 0, .7]
        color_xyz = xyz + [.1, 0, 0]
        uv = color_xyz[:, :2] / color_xyz[:, 2:3] * 300 + [320, 240]
        low, high = uv.min(axis=0) - 1, uv.max(axis=0) + 1
        # 框外背景，不应进入发布的目标云。
        xyz = np.vstack([xyz, [1.5, 0, .7]])
        rgb = np.tile([20, 150, 80], (len(xyz), 1))
        cloud = make_cloud(Header(stamp=stamp.to_msg(), frame_id=frame), xyz, rgb)
        detections = Detection2DArray()
        detections.header = Header(stamp=Time(nanoseconds=stamp.nanoseconds + int(offset * 1e9)).to_msg(),
                                   frame_id='color')
        if not empty:
            detection = Detection2D()
            detection.header = detections.header
            detection.bbox.center.position.x, detection.bbox.center.position.y = ((low + high) / 2).tolist()
            detection.bbox.size_x, detection.bbox.size_y = (high - low).tolist()
            detections.detections.append(detection)
        self.cloud_pub.publish(cloud)
        self.det_pub.publish(detections)
        return cloud

    def test_tf_crop_registration_and_empty_detection(self):
        cloud = self.send(offset=.01)
        self.spin_until(lambda: len(self.messages['poses']) > 0)
        poses = self.messages['poses'][-1]
        self.assertEqual(len(poses.poses), 1)
        self.assertEqual(poses.header.frame_id, 'depth')
        self.assertEqual(poses.header.stamp, cloud.header.stamp)
        position = poses.poses[0].position
        np.testing.assert_allclose([position.x, position.y, position.z], [-.1, 0, .7], atol=.005)
        target_xyz, rgb = read_cloud(self.messages['target'][-1])
        roi_xyz, roi_rgb = read_cloud(self.messages['roi'][-1])
        self.assertEqual(len(roi_xyz), len(self.registration.source_xyz))
        np.testing.assert_allclose(roi_xyz, self.registration.source_xyz + [-.1, 0, .7], atol=1e-7)
        np.testing.assert_array_equal(roi_rgb, np.tile([20, 150, 80], (len(roi_rgb), 1)))
        self.assertGreater(len(target_xyz), 80)
        self.assertLess(target_xyz[:, 0].max(), .1)  # 框外背景被排除
        np.testing.assert_array_equal(rgb, np.tile([20, 150, 80], (len(rgb), 1)))
        self.assertGreater(self.messages['aligned'][-1].width, 80)
        self.assertEqual(self.messages['model'][-1].header.frame_id, 'energy_model')
        count = len(self.messages['poses'])
        self.send(empty=True)
        self.spin_until(lambda: len(self.messages['poses']) > count)
        self.assertEqual(len(self.messages['poses'][-1].poses), 0)
        self.assertEqual(self.messages['target'][-1].width, 0)
        self.assertEqual(self.messages['roi'][-1].width, 0)
        self.assertEqual(self.messages['aligned'][-1].width, 0)

    def test_unmatched_frames_missing_tf_and_old_data(self):
        self.send(offset=.2)
        self.spin_for(.15)
        self.assertEqual(len(self.messages['poses']), 0)
        self.send(frame='unknown_depth')
        self.spin_until(lambda: len(self.messages['poses']) > 0)
        self.assertEqual(len(self.messages['poses'][-1].poses), 0)
        # 清除同步缓存，单独核对过期输入；与前一个目标无关系。
        self.registration.last_stamp_ns = -1
        count = len(self.messages['poses'])
        self.send(age=2.)
        self.spin_until(lambda: len(self.messages['poses']) > count)
        self.assertEqual(self.messages['aligned'][-1].width, 0)
        self.assertEqual(len(self.messages['poses'][-1].poses), 0)

    def test_watchdog_clears_after_stream_stops(self):
        self.send()
        self.spin_until(lambda: len(self.messages['poses']) > 0)
        self.assertEqual(len(self.messages['poses'][-1].poses), 1)
        self.spin_until(lambda: len(self.messages['poses']) >= 2, timeout=2.)
        self.assertEqual(len(self.messages['poses'][-1].poses), 0)
        self.assertEqual(self.messages['target'][-1].width, 0)
        self.assertEqual(self.messages['roi'][-1].width, 0)
        self.assertEqual(self.messages['aligned'][-1].width, 0)


if __name__ == '__main__':
    unittest.main()
