"""将时间接近的点云变换到彩色光学系，再投影到原图；不等待 ICP。"""
from collections import deque
import math

import cv2
import numpy as np
from rclpy.qos import QoSProfile, ReliabilityPolicy
from rclpy.time import Time
from sensor_msgs.msg import CameraInfo, PointCloud2, PointField
from tf2_ros import Buffer, TransformException, TransformListener


def stamp_seconds(stamp):
    return stamp.sec + stamp.nanosec * 1e-9


def cloud_xyz(cloud, max_points):
    """尊重行填充、字段偏移和字节序，先限点再分配 XYZ 数组。"""
    if not cloud.width or not cloud.height:
        return np.empty((0, 3), dtype=np.float64)
    fields = {field.name: field for field in cloud.fields}
    formats = {PointField.FLOAT32: 'f4', PointField.FLOAT64: 'f8'}
    if any(name not in fields or fields[name].datatype not in formats for name in ('x', 'y', 'z')):
        raise ValueError('点云必须包含浮点 x/y/z 字段')
    endian = '>' if cloud.is_bigendian else '<'
    dtype = np.dtype({'names': ['x', 'y', 'z'],
                      'formats': [endian + formats[fields[n].datatype] for n in ('x', 'y', 'z')],
                      'offsets': [fields[n].offset for n in ('x', 'y', 'z')],
                      'itemsize': cloud.point_step})
    view = np.ndarray((cloud.height, cloud.width), dtype=dtype, buffer=cloud.data,
                      strides=(cloud.row_step, cloud.point_step))
    indices = np.arange(0, cloud.width * cloud.height,
                        max(1, math.ceil(cloud.width * cloud.height / max_points)))
    samples = view[indices // cloud.width, indices % cloud.width]
    return np.column_stack([samples[n] for n in ('x', 'y', 'z')]).astype(np.float64)


def transform_xyz(points, transform):
    q = transform.rotation
    x, y, z, w = np.array([q.x, q.y, q.z, q.w]) / np.linalg.norm([q.x, q.y, q.z, q.w])
    rotation = np.array([[1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
                         [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
                         [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)]])
    t = transform.translation
    return points @ rotation.T + [t.x, t.y, t.z]


def project_pixels(points, camera, width, height):
    points = points[np.isfinite(points).all(axis=1) & (points[:, 2] > 1e-6)]
    if not len(points):
        return np.empty((0, 2), dtype=int)
    k = np.asarray(camera.k, dtype=np.float64).reshape(3, 3)
    distortion = np.asarray(camera.d, dtype=np.float64)
    if camera.distortion_model == 'equidistant':
        pixels, _ = cv2.fisheye.projectPoints(points.reshape(-1, 1, 3), np.zeros(3),
                                             np.zeros(3), k, distortion)
    elif camera.distortion_model in ('', 'plumb_bob', 'rational_polynomial'):
        pixels, _ = cv2.projectPoints(points, np.zeros(3), np.zeros(3), k, distortion)
    else:
        raise ValueError(f'不支持的相机畸变模型: {camera.distortion_model}')
    pixels = pixels.reshape(-1, 2)
    valid = (np.isfinite(pixels).all(axis=1) & (pixels[:, 0] >= 0)
             & (pixels[:, 0] < width) & (pixels[:, 1] >= 0) & (pixels[:, 1] < height))
    return np.floor(pixels[valid]).astype(int)


class CloudOverlay:
    def __init__(self, node):
        self.node = node
        self.enabled = node.declare_parameter('overlay_cloud_enabled', True).value
        topic = node.declare_parameter('overlay_cloud_topic', '/vision/icp/aligned_cloud').value
        camera_topic = node.declare_parameter('overlay_camera_info_topic', '/camera/camera/color/camera_info').value
        self.max_age = float(node.declare_parameter('overlay_cloud_max_age', 1.5).value)
        self.max_points = node.declare_parameter('overlay_cloud_max_points', 3000).value
        self.radius = node.declare_parameter('overlay_cloud_radius', 1).value
        if (not math.isfinite(self.max_age) or self.max_age <= 0
                or not isinstance(self.max_points, int) or self.max_points <= 0
                or not isinstance(self.radius, int) or not 1 <= self.radius <= 5):
            raise ValueError('投影时间容差/点数必须为正，点半径必须是 1~5 的整数')
        self.clouds = deque(maxlen=8)
        self.camera = None
        if self.enabled:
            qos = QoSProfile(depth=1, reliability=ReliabilityPolicy.BEST_EFFORT)
            self.cloud_sub = node.create_subscription(PointCloud2, topic, self.clouds.append, qos)
            self.camera_sub = node.create_subscription(CameraInfo, camera_topic, self.receive_camera, qos)
            self.tf_buffer = Buffer()
            self.tf_listener = TransformListener(self.tf_buffer, node)
            node.get_logger().info(f'图像点云投影: {topic}，最多 {self.max_points} 点，时间容差 {self.max_age}s')

    def receive_camera(self, message):
        self.camera = message

    def draw(self, image, header):
        if not self.enabled or not self.clouds or self.camera is None:
            return image
        camera = self.camera
        if (camera.header.frame_id != header.frame_id or camera.width != image.shape[1]
                or camera.height != image.shape[0] or camera.k[0] <= 0 or camera.k[4] <= 0):
            self.node.get_logger().warning('投影跳过：图像与 CameraInfo 的坐标系/尺寸/内参不匹配',
                                           throttle_duration_sec=5.0)
            return image
        cloud = min(self.clouds, key=lambda c: abs(stamp_seconds(c.header.stamp) - stamp_seconds(header.stamp)))
        if abs(stamp_seconds(cloud.header.stamp) - stamp_seconds(header.stamp)) > self.max_age:
            return image
        try:
            points = cloud_xyz(cloud, self.max_points)
            if not len(points):
                return image
            if cloud.header.frame_id != camera.header.frame_id:
                transform = self.tf_buffer.lookup_transform(
                    camera.header.frame_id, cloud.header.frame_id, Time.from_msg(cloud.header.stamp))
                points = transform_xyz(points, transform.transform)
            pixels = project_pixels(points, camera, image.shape[1], image.shape[0])
            output = image.copy()
            for u, v in pixels:
                cv2.circle(output, (int(u), int(v)), self.radius, (0, 80, 255), -1)
            return output
        except (TransformException, ValueError, cv2.error) as error:
            self.node.get_logger().warning(f'点云投影暂不可用: {error}', throttle_duration_sec=5.0)
            return image
