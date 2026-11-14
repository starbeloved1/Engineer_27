"""独立 ROS 域回验模型、配准、精确同帧画框和失效清空；不启动相机。"""
import os
from contextlib import contextmanager
import tempfile
import unittest
from pathlib import Path
import signal
import subprocess
import time

import numpy as np
from ament_index_python.packages import get_package_prefix, get_package_share_directory
import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, QoSProfile
from geometry_msgs.msg import PoseArray
from sensor_msgs.msg import CameraInfo, Image, PointCloud2
from sensor_msgs_py import point_cloud2
from std_msgs.msg import Header
from vision_msgs.msg import Detection2D, Detection2DArray, ObjectHypothesisWithPose


def xyz(message):
    values = list(point_cloud2.read_points(message, field_names=('x', 'y', 'z'), skip_nans=True))
    return np.asarray([[p[0], p[1], p[2]] for p in values], dtype=float).reshape(-1, 3)


@contextmanager
def pipeline_context(tmp_path):
    # 避免与用户正在运行的视觉节点、TF 和显示端混在同一域。
    original_domain = os.environ.get('ROS_DOMAIN_ID')
    os.environ['ROS_DOMAIN_ID'] = '196'
    share = Path(get_package_share_directory('vision_nodes_pcl'))
    binaries = Path(get_package_prefix('vision_nodes_pcl')) / 'lib/vision_nodes_pcl'
    processes, logs = [], []
    rclpy.init()
    peer = Node('pcl_pipeline_test')
    try:
        for name in ('model_cloud_publisher', 'icp_registration', 'main_visualizer'):
            log = (tmp_path / f'{name}.log').open('w+')
            logs.append(log)
            command = [str(binaries / name), '--ros-args', '--params-file', str(share / 'config/vision_params.yaml')]
            if name == 'icp_registration':
                command += ['-p', 'target_frame:=test_optical', '-p', 'max_data_age:=0.8', '-p', 'publish_tf:=false']
            processes.append(subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT))
        yield peer
    finally:
        for process in processes:
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
        for process in processes:
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        peer.destroy_node()
        rclpy.shutdown()
        for log in logs:
            log.seek(0)
            print(log.read())
            log.close()
        if original_domain is None:
            os.environ.pop('ROS_DOMAIN_ID', None)
        else:
            os.environ['ROS_DOMAIN_ID'] = original_domain
        assert all(p.returncode == 0 for p in processes), 'PCL 节点异常退出，见上述日志'


def run_pipeline_checks(peer):
    received = {key: [] for key in ('model', 'roi', 'target', 'aligned', 'raw_aligned', 'poses', 'raw_poses', 'image')}
    subscriptions = []
    for key, topic, kind in (
            ('model', '/vision/model_cloud', PointCloud2),
            ('roi', '/vision/icp/roi_cloud', PointCloud2),
            ('target', '/vision/icp/target_cloud', PointCloud2),
            ('aligned', '/vision/icp/aligned_cloud', PointCloud2),
            ('raw_aligned', '/vision/icp/raw_aligned_cloud', PointCloud2),
            ('poses', '/vision/icp/object_poses', PoseArray),
            ('raw_poses', '/vision/icp/raw_object_poses', PoseArray),
            ('image', '/vision/visualization/image', Image)):
        qos = QoSProfile(depth=10, durability=(DurabilityPolicy.TRANSIENT_LOCAL
                          if key == 'model' else DurabilityPolicy.VOLATILE))
        subscriptions.append(peer.create_subscription(kind, topic, lambda msg, k=key: received[k].append(msg), qos))
    cloud_pub = peer.create_publisher(PointCloud2, '/camera/camera/depth/color/points', 10)
    camera_pub = peer.create_publisher(CameraInfo, '/camera/camera/color/camera_info', 10)
    detection_pub = peer.create_publisher(Detection2DArray, '/vision/yolo/detections', 10)
    image_pub = peer.create_publisher(Image, '/camera/camera/color/image_raw', 10)

    def wait(predicate, timeout=5):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            rclpy.spin_once(peer, timeout_sec=0.02)
            if predicate():
                return
        raise AssertionError('等待 ROS 输出超时')

    wait(lambda: received['model'] and cloud_pub.get_subscription_count() > 0
         and detection_pub.get_subscription_count() == 2 and image_pub.get_subscription_count() > 0)
    points = xyz(received['model'][-1])
    angle = 0.04
    rotation = np.array([[np.cos(angle), -np.sin(angle), 0], [np.sin(angle), np.cos(angle), 0], [0, 0, 1]])
    translation = np.array([0.03, -0.02, 0.6])
    observed = points @ rotation.T + translation

    def publish(empty=False):
        header = Header(stamp=peer.get_clock().now().to_msg(), frame_id='test_optical')
        info = CameraInfo(header=header, width=640, height=480)
        info.k = [300., 0., 320., 0., 300., 240., 0., 0., 1.]
        camera_pub.publish(info)
        detections = Detection2DArray(header=header)
        if not empty:
            detection = Detection2D(header=header)
            detection.bbox.center.position.x = 320.
            detection.bbox.center.position.y = 240.
            detection.bbox.size_x, detection.bbox.size_y = 640., 480.
            hypothesis = ObjectHypothesisWithPose()
            hypothesis.hypothesis.class_id, hypothesis.hypothesis.score = '0', 0.95
            detection.results.append(hypothesis)
            detections.detections.append(detection)
        detection_pub.publish(detections)
        image_pub.publish(Image(header=header, width=640, height=480, encoding='bgr8',
                                step=640 * 3, data=bytes(640 * 480 * 3)))
        cloud_pub.publish(point_cloud2.create_cloud_xyz32(header, observed.tolist()))
        return header

    for _ in range(10):
        publish()
        end = time.monotonic() + 0.12
        while time.monotonic() < end:
            rclpy.spin_once(peer, timeout_sec=0.02)
        if received['raw_poses'] and received['raw_poses'][-1].poses and received['image']:
            break
    wait(lambda: received['raw_poses'] and received['raw_poses'][-1].poses and received['image'])
    raw = received['raw_poses'][-1].poses[0]
    np.testing.assert_allclose([raw.position.x, raw.position.y, raw.position.z], translation, atol=0.006)
    assert received['target'][-1].width >= 80
    assert received['aligned'][-1].width > 0 and received['raw_aligned'][-1].width > 0
    assert received['target'][-1].header.frame_id == 'test_optical'
    assert 'rgb' in [field.name for field in received['roi'][-1].fields]
    assert np.frombuffer(bytes(received['image'][-1].data), dtype=np.uint8).max() > 0
    # Z 轴输出抹掉绕 Z 的旋转，但 raw_pose 保留原始 ICP 朝向。
    assert abs(raw.orientation.z) > 0.005
    assert abs(received['poses'][-1].poses[0].orientation.z) < 0.005

    # 空检测必须清空本帧全部结果，不能停留在上一帧。
    header = publish(empty=True)
    wait(lambda: received['poses'][-1].header.stamp == header.stamp and not received['poses'][-1].poses)
    wait(lambda: received['aligned'][-1].header.stamp == header.stamp and received['aligned'][-1].width == 0)
    assert received['roi'][-1].width == 0
    # 恢复检测，随后断流，watchdog 应主动清空，而不是等待下一帧。
    publish()
    wait(lambda: received['poses'][-1].poses)
    wait(lambda: not received['poses'][-1].poses, timeout=3)
    wait(lambda: received['target'][-1].width == 0)
    # 可视化使用精确时间戳配对：单独一张没有对应检测的图不生成输出。
    old_count = len(received['image'])
    header = Header(stamp=peer.get_clock().now().to_msg(), frame_id='test_optical')
    image_pub.publish(Image(header=header, width=640, height=480, encoding='bgr8',
                            step=640 * 3, data=bytes(640 * 480 * 3)))
    end = time.monotonic() + 0.2
    while time.monotonic() < end:
        rclpy.spin_once(peer, timeout_sec=0.02)
    assert len(received['image']) == old_count
    detection_pub.publish(Detection2DArray(header=header))
    wait(lambda: received['image'][-1].header.stamp == header.stamp)
    assert not any(received['image'][-1].data)


class PipelineTests(unittest.TestCase):
    def test_model_icp_visualization_and_invalidation(self):
        with tempfile.TemporaryDirectory(prefix='vision_pcl_test_') as directory:
            with pipeline_context(Path(directory)) as peer:
                run_pipeline_checks(peer)


if __name__ == '__main__':
    unittest.main()
