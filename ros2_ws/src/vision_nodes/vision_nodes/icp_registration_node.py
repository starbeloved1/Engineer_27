"""YOLO 框内点云提取与矿模型局部配准；不控制机械臂。"""
import math
import sys
import time

from geometry_msgs.msg import Pose, PoseArray
from message_filters import ApproximateTimeSynchronizer, Subscriber
import numpy as np
import rclpy
from rcl_interfaces.msg import ParameterDescriptor
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, QoSProfile, ReliabilityPolicy
from rclpy.time import Time
from scipy.spatial.transform import Rotation
from sensor_msgs.msg import CameraInfo, PointCloud2
from std_msgs.msg import Header
from tf2_ros import Buffer, TransformException, TransformListener
from vision_msgs.msg import Detection2DArray

from .pointcloud_geometry import (
    accepted_registration, filtered_cloud, load_model, register_model, roi_mask,
)
from .pointcloud_messages import make_cloud, read_cloud


class IcpRegistration(Node):
    def __init__(self):
        super().__init__('icp_registration')
        defaults = {
            'pointcloud_topic': '/camera/camera/depth/color/points',
            'camera_info_topic': '/camera/camera/color/camera_info',
            'detection_topic': '/vision/yolo/detections',
            'source_cloud_topic': '/vision/model_cloud',
            'roi_cloud_topic': '/vision/icp/roi_cloud',
            'target_cloud_topic': '/vision/icp/target_cloud',
            'aligned_cloud_topic': '/vision/icp/aligned_cloud',
            'pose_topic': '/vision/icp/object_poses',
            'model_path': 'assets/energy.stl', 'model_scale': 0.001,
            'model_frame': 'energy_model', 'sample_points': 2000, 'model_voxel_size': 0.005,
            'voxel_size': 0.005, 'min_depth': 0.2, 'max_depth': 2.0,
            'bbox_shrink_ratio': 0.0, 'min_points': 80,
            'sor_neighbors': 20, 'sor_std_ratio': 1.5,
            'max_corr_dist': 0.03, 'max_icp_iter': 100,
            'fitness_threshold': 0.3, 'max_inlier_rmse': 0.015,
            'initial_rpy_deg': [0.0, 0.0, 0.0],
            'sync_queue_size': 30, 'sync_slop': 0.05,
            'max_data_age': 1.5, 'input_reliability': 'reliable', 'log_every_n': 10,
        }
        # YAML 中的 0 是整数，允许该参数同时接受整数 0 和浮点 0.0。
        self.params = {}
        for name, value in defaults.items():
            if name == 'max_inlier_rmse':
                parameter = self.declare_parameter(
                    name, value, ParameterDescriptor(dynamic_typing=True))
            else:
                parameter = self.declare_parameter(name, value)
            self.params[name] = parameter.value
        p = self.params
        if (isinstance(p['max_inlier_rmse'], bool)
                or not isinstance(p['max_inlier_rmse'], (int, float))):
            raise ValueError('max_inlier_rmse 必须为有限非负数，0 禁用残差上限')
        for name in ('model_scale', 'max_depth', 'max_corr_dist',
                     'sor_std_ratio', 'sync_slop', 'max_data_age'):
            if not math.isfinite(p[name]) or p[name] <= 0:
                raise ValueError(f'{name} 必须为有限正数')
        for name in ('model_voxel_size', 'voxel_size', 'min_depth', 'max_inlier_rmse'):
            if not math.isfinite(p[name]) or p[name] < 0:
                raise ValueError(f'{name} 必须为有限非负数')
        if (p['min_depth'] >= p['max_depth']
                or not 0 <= p['bbox_shrink_ratio'] < 1
                or not 0 < p['fitness_threshold'] <= 1):
            raise ValueError('深度范围、框收缩比例或 fitness 阈值无效')
        if any(p[name] < 3 for name in ('sample_points', 'min_points')):
            raise ValueError('模型与目标点数至少为 3')
        if any(p[name] <= 0 for name in ('max_icp_iter', 'sync_queue_size')):
            raise ValueError('迭代数与同步队列长度必须为正数')
        if p['sor_neighbors'] < 0 or p['log_every_n'] < 0:
            raise ValueError('sor_neighbors、log_every_n 不能为负数')
        if len(p['initial_rpy_deg']) != 3 or not np.isfinite(p['initial_rpy_deg']).all():
            raise ValueError('initial_rpy_deg 必须为三个有限角度，单位度')
        if p['input_reliability'] not in ('reliable', 'best_effort'):
            raise ValueError('input_reliability 必须为 reliable 或 best_effort')
        for name in (*[key for key in p if key.endswith('_topic')], 'model_frame'):
            if not p[name]:
                raise ValueError(f'{name} 不能为空')
        outputs = [self.resolve_topic_name(p[name]) for name in (
            'source_cloud_topic', 'roi_cloud_topic', 'target_cloud_topic', 'aligned_cloud_topic', 'pose_topic')]
        inputs = [self.resolve_topic_name(p[name]) for name in (
            'pointcloud_topic', 'camera_info_topic', 'detection_topic')]
        if len(set(outputs)) != len(outputs) or set(outputs) & set(inputs):
            raise ValueError('输出话题必须互不相同，且不能与输入话题相同')

        self.source = load_model(p['model_path'], p['model_scale'], p['sample_points'],
                                 p['model_voxel_size'])
        if len(self.source.points) < p['min_points']:
            raise ValueError('模型下采样后点数不足，请检查单位和 model_voxel_size')
        self.source_xyz = np.asarray(self.source.points)
        self.camera_info = None
        self.last_stamp_ns = -1
        self.last_output_at = None
        self.last_header = None
        self.frame_count = 0
        self.stale_cleared = True
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        qos = QoSProfile(depth=1, reliability=ReliabilityPolicy.RELIABLE)
        self.roi_pub = self.create_publisher(PointCloud2, p['roi_cloud_topic'], qos)
        self.target_pub = self.create_publisher(PointCloud2, p['target_cloud_topic'], qos)
        self.aligned_pub = self.create_publisher(PointCloud2, p['aligned_cloud_topic'], qos)
        self.pose_pub = self.create_publisher(PoseArray, p['pose_topic'], qos)
        self.model_pub = self.create_publisher(PointCloud2, p['source_cloud_topic'], QoSProfile(
            depth=1, reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL))
        # 原始模型在自己的坐标系，不伪造它到相机的单位变换。
        self.model_pub.publish(make_cloud(
            Header(stamp=self.get_clock().now().to_msg(), frame_id=p['model_frame']),
            self.source_xyz))
        input_qos = QoSProfile(depth=1, reliability=(ReliabilityPolicy.RELIABLE
                              if p['input_reliability'] == 'reliable'
                              else ReliabilityPolicy.BEST_EFFORT))
        self.info_sub = self.create_subscription(
            CameraInfo, p['camera_info_topic'], self.info_callback, input_qos)
        self.cloud_sub = Subscriber(self, PointCloud2, p['pointcloud_topic'], qos_profile=input_qos)
        self.detection_sub = Subscriber(self, Detection2DArray, p['detection_topic'],
                                        qos_profile=QoSProfile(depth=10))
        # 缓存云等待推理，按采集时间近似配对；不把 latest_detections 用于未来帧。
        self.synchronizer = ApproximateTimeSynchronizer(
            [self.cloud_sub, self.detection_sub], p['sync_queue_size'], p['sync_slop'])
        self.synchronizer.registerCallback(self.process_pair)
        self.watchdog = self.create_timer(0.25, self.clear_stale)
        self.get_logger().info(
            f"ICP ready: model={p['model_path']}, scale={p['model_scale']}, "
            f"sampling=poisson_disk, sample_points={p['sample_points']}, "
            f"model_voxel_size={p['model_voxel_size']} (源模型单位), "
            f"points={len(self.source_xyz)}, extent_m={np.ptp(self.source_xyz, axis=0)}; "
            f"sync_slop={p['sync_slop']}s; 局部配准，尚未验证实物位姿精度")

    def info_callback(self, message):
        matrix = np.asarray(message.k).reshape(3, 3)
        if (message.width <= 0 or message.height <= 0 or not message.header.frame_id
                or not np.isfinite(matrix).all() or matrix[0, 0] <= 0 or matrix[1, 1] <= 0
                or not np.isfinite(message.d).all()
                or message.distortion_model not in ('plumb_bob', 'rational_polynomial', '')
                or len(message.d) not in (0, 4, 5, 8, 12, 14)):
            self.camera_info = None
            self.get_logger().warning('相机内参无效或畸变模型不支持', throttle_duration_sec=5.0)
            return
        self.camera_info = message

    def cloud_to_color(self, cloud_frame, color_frame, stamp):
        matrix = np.eye(4)
        if cloud_frame == color_frame:
            return matrix
        transform = self.tf_buffer.lookup_transform(color_frame, cloud_frame, Time.from_msg(stamp))
        q, t = transform.transform.rotation, transform.transform.translation
        matrix[:3, :3] = Rotation.from_quat([q.x, q.y, q.z, q.w]).as_matrix()
        matrix[:3, 3] = [t.x, t.y, t.z]
        return matrix

    def publish_result(self, header, targets, aligned, poses, raw_targets=()):
        raw_xyz = np.concatenate([xyz for xyz, _ in raw_targets]) if raw_targets else np.empty((0, 3))
        raw_rgb = (np.concatenate([rgb for _, rgb in raw_targets])
                   if raw_targets and all(rgb is not None for _, rgb in raw_targets) else None)
        self.roi_pub.publish(make_cloud(header, raw_xyz, raw_rgb))
        xyz = np.concatenate([np.asarray(cloud.points) for cloud in targets]) if targets else np.empty((0, 3))
        rgb = None
        if targets and all(cloud.has_colors() for cloud in targets):
            rgb = np.rint(np.concatenate([np.asarray(cloud.colors) for cloud in targets]) * 255)
        self.target_pub.publish(make_cloud(header, xyz, rgb))
        self.aligned_pub.publish(make_cloud(
            header, np.concatenate(aligned) if aligned else np.empty((0, 3))))
        self.pose_pub.publish(PoseArray(header=header, poses=poses))
        self.last_header = header
        self.last_output_at = time.monotonic()
        self.stale_cleared = not (raw_targets or targets or aligned or poses)

    def clear_stale(self):
        if (not self.stale_cleared and self.last_output_at is not None
                and time.monotonic() - self.last_output_at > self.params['max_data_age']):
            header = Header(stamp=self.get_clock().now().to_msg(),
                            frame_id=self.last_header.frame_id)
            self.publish_result(header, [], [], [])
            self.get_logger().warning('未收到新鲜的配对数据，已清空目标云和位姿')

    def process_pair(self, cloud, detections):
        p = self.params
        stamp_ns = Time.from_msg(cloud.header.stamp).nanoseconds
        if stamp_ns <= self.last_stamp_ns:
            return
        self.last_stamp_ns = stamp_ns
        self.frame_count += 1
        now = self.get_clock().now().nanoseconds
        ages = [(now - Time.from_msg(header.stamp).nanoseconds) / 1e9
                for header in (cloud.header, detections.header)]
        if max(ages) > p['max_data_age'] or min(ages) < -p['sync_slop']:
            self.publish_result(cloud.header, [], [], [])
            self.get_logger().warning('忽略过期或未来时间戳的输入', throttle_duration_sec=5.0)
            return
        if not detections.detections:
            self.publish_result(cloud.header, [], [], [])
            return
        info = self.camera_info
        if info is None or info.header.frame_id != detections.header.frame_id or not cloud.header.frame_id:
            self.publish_result(cloud.header, [], [], [])
            self.get_logger().warning('等待匹配彩色坐标系的 CameraInfo', throttle_duration_sec=5.0)
            return
        try:
            transform = self.cloud_to_color(cloud.header.frame_id, info.header.frame_id, cloud.header.stamp)
        except (TransformException, ValueError) as error:
            self.publish_result(cloud.header, [], [], [])
            self.get_logger().warning(f'缺少点云到彩色相机的有效 TF: {error}', throttle_duration_sec=5.0)
            return
        try:
            xyz, rgb = read_cloud(cloud)
        except (ValueError, TypeError, KeyError, AssertionError) as error:
            self.publish_result(cloud.header, [], [], [])
            self.get_logger().error(f'点云解析失败: {error}', throttle_duration_sec=5.0)
            return
        targets, aligned, poses, raw_targets = [], [], [], []
        log_frame = (self.frame_count == 1
                     or (p['log_every_n'] and self.frame_count % p['log_every_n'] == 0))
        for index, detection in enumerate(detections.detections):
            if (detection.header.frame_id != detections.header.frame_id
                    or detection.header.stamp != detections.header.stamp):
                continue
            box = detection.bbox
            mask = roi_mask(
                xyz, np.asarray(info.k).reshape(3, 3), np.asarray(info.d), transform,
                (box.center.position.x, box.center.position.y, box.size_x, box.size_y, box.center.theta),
                (info.width, info.height), p['min_depth'], p['max_depth'], p['bbox_shrink_ratio'])
            if not mask.any():
                if log_frame:
                    self.get_logger().info(
                        f'ROI[{index}]: scene={len(xyz)}, raw=0; '
                        '框内无有效点，请检查深度、距离范围、框与投影')
                continue
            roi_xyz, roi_rgb = xyz[mask], rgb[mask] if rgb is not None else None
            raw_targets.append((roi_xyz, roi_rgb))
            counts = {}
            target = filtered_cloud(roi_xyz, roi_rgb, p['voxel_size'],
                                    p['sor_neighbors'], p['sor_std_ratio'], statistics=counts)
            count_text = (f"scene={len(xyz)}, raw={counts['raw']}, "
                          f"voxel={counts['voxel']}, sor={counts['sor']}")
            # 无论 ICP 是否通过，都发布筛选后的 ROI，便于先检查输入是否正确。
            if len(target.points):
                targets.append(target)
            if len(target.points) < p['min_points']:
                if log_frame:
                    self.get_logger().info(
                        f"ROI[{index}]: {count_text}; 点数不足 {p['min_points']}，跳过 ICP")
                continue
            result = register_model(self.source, target, p['initial_rpy_deg'],
                                    p['max_corr_dist'], p['max_icp_iter'])
            accepted = accepted_registration(result, p['fitness_threshold'],
                                             p['max_inlier_rmse'], p['min_points'])
            if log_frame:
                self.get_logger().info(
                    f'ROI[{index}]: {count_text}, fitness={result.fitness:.3f}, '
                    f'rmse={result.inlier_rmse:.4f}m, accepted={accepted}')
            if not accepted:
                continue
            matrix = result.transformation
            aligned.append(self.source_xyz @ matrix[:3, :3].T + matrix[:3, 3])
            pose = Pose()
            pose.position.x, pose.position.y, pose.position.z = matrix[:3, 3].tolist()
            q = Rotation.from_matrix(matrix[:3, :3]).as_quat()
            pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w = q.tolist()
            poses.append(pose)
        self.publish_result(cloud.header, targets, aligned, poses, raw_targets)


def main(args=None):
    rclpy.init(args=args)
    node = None
    try:
        node = IcpRegistration()
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    except Exception as error:
        print(f'icp_registration failed: {error}', file=sys.stderr)
        return 1
    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
    return 0


if __name__ == '__main__':
    sys.exit(main())
