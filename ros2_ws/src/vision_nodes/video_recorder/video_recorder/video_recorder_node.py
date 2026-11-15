"""只订阅彩色原图录制 MP4，不保存深度、点云或检测框。"""
from datetime import datetime
import math
from pathlib import Path
import sys
from zoneinfo import ZoneInfo

from ament_index_python.packages import get_package_share_directory
import cv2
from cv_bridge import CvBridge, CvBridgeError
import rclpy
from rcl_interfaces.msg import ParameterDescriptor
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Image
import yaml


def resolve_output_directory(directory):
    path = Path(directory).expanduser()
    if path.is_absolute():
        return path.resolve()
    # symlink-install 和普通工作区安装都从包所在路径向上找工程根目录，独立于启动 cwd。
    for parent in Path(__file__).resolve().parents:
        if (parent / 'run.sh').is_file() and (parent / 'ros2_ws').is_dir():
            return (parent / path).resolve()
    raise ValueError('无法确定 Engineer_27 根目录，请将录制 output_dir 配置为绝对路径')


def color_fps(camera_config):
    with Path(camera_config).expanduser().open(encoding='utf-8') as stream:
        config = yaml.safe_load(stream)
    try:
        profile = config['/camera/camera']['ros__parameters']['rgb_camera.color_profile']
        parts = profile.lower().replace(',', 'x').split('x')
        if len(parts) != 3:
            raise ValueError('彩色 profile 需要 width x height x fps')
        fps = float(parts[2])
    except (KeyError, TypeError, AttributeError, ValueError) as error:
        raise ValueError('无法从 camera.yaml 的 rgb_camera.color_profile 读取录制帧率') from error
    if not math.isfinite(fps) or fps <= 0:
        raise ValueError('彩色 profile 需要明确的正帧率，或为 video_recorder 单独设置 fps')
    return fps


class VideoRecorder(Node):
    def __init__(self):
        super().__init__('video_recorder')
        self.declare_parameter('video', False)
        self.color_topic = self.declare_parameter(
            'color_topic', '/camera/camera/color/image_raw').value
        output_dir = self.declare_parameter('output_dir', 'video').value
        fps = self.declare_parameter('fps', 0.0, ParameterDescriptor(dynamic_typing=True)).value
        camera_config = self.declare_parameter(
            'camera_config', str(Path(get_package_share_directory('camera_driver'))
                                 / 'config/camera.yaml')).value
        if (not output_dir or not self.color_topic or isinstance(fps, bool)
                or not isinstance(fps, (int, float)) or not math.isfinite(fps) or fps < 0):
            raise ValueError('录制话题/目录不能为空，fps 必须为有限非负值')
        self.output_dir = resolve_output_directory(output_dir)
        self.fps = float(fps) if fps > 0 else None
        self.camera_config = camera_config
        self.bridge = CvBridge()
        self.subscription = None
        self.enabled = False
        self.writer = None
        self.size = None
        self.path = None
        self.frame_count = 0
        self.timer = self.create_timer(0.1, self.update_recording)
        self.update_recording()
        self.get_logger().info(
            f'RGB recorder: video={self.enabled}, {self.color_topic}, '
            f'fps={self.fps or "camera profile"}, output={self.output_dir}')

    def update_recording(self):
        enabled = self.get_parameter('video').value
        if enabled == self.enabled:
            return
        self.enabled = enabled
        if enabled:
            if self.fps is None:
                self.fps = color_fps(self.camera_config)
            self.subscription = self.create_subscription(
                Image, self.color_topic, self.image_callback,
                QoSProfile(depth=1, reliability=ReliabilityPolicy.RELIABLE))
            self.get_logger().info('已开启 RGB 录制，等待彩色图像')
        else:
            if self.subscription is not None:
                self.destroy_subscription(self.subscription)
                self.subscription = None
            self.close_video()
            self.get_logger().info('已关闭 RGB 录制')

    def open_video(self, width, height):
        self.output_dir.mkdir(parents=True, exist_ok=True)
        timestamp = datetime.now(ZoneInfo('Asia/Shanghai')).strftime('%Y%m%d_%H%M%S_%f')
        # 排他创建，只保留本次录制的文件名，绝不覆盖已有视频。
        for suffix in range(1000):
            ending = f'_{suffix}' if suffix else ''
            path = self.output_dir / f'rgb_{timestamp}{ending}.mp4'
            try:
                path.touch(exist_ok=False)
                break
            except FileExistsError:
                continue
        else:
            raise RuntimeError('无法分配唯一的视频文件名')
        writer = cv2.VideoWriter(str(path), cv2.VideoWriter_fourcc(*'mp4v'),
                                 self.fps, (width, height))
        if not writer.isOpened():
            writer.release()
            path.unlink()  # 仅删除本次排他创建的空文件。
            raise RuntimeError(f'MP4 编码器启动失败: {path}')
        self.writer, self.path = writer, path
        self.size, self.frame_count = (width, height), 0
        self.get_logger().info(f'开始录制: {path} ({width}x{height}, {self.fps:g} FPS)')

    def close_video(self):
        if self.writer is not None:
            self.writer.release()  # 写完容器尾部，保证正常停止后可播放。
            self.writer = None
            self.get_logger().info(f'视频已保存: {self.path}, {self.frame_count} 帧')
        self.size = None

    def image_callback(self, message):
        if not self.get_parameter('video').value:
            self.update_recording()
            return
        try:
            # VideoWriter 使用 BGR；只转换通道顺序，不叠加任何检测或文字。
            image = self.bridge.imgmsg_to_cv2(message, desired_encoding='bgr8')
        except CvBridgeError as error:
            self.get_logger().error(f'录制图像转换失败: {error}', throttle_duration_sec=5.0)
            return
        height, width = image.shape[:2]
        if width <= 0 or height <= 0:
            return
        if self.writer is not None and self.size != (width, height):
            self.close_video()  # 分辨率变化时另开文件，不向旧尺寸的编码器写入。
        if self.writer is None:
            self.open_video(width, height)
        self.writer.write(image)
        self.frame_count += 1

    def destroy_node(self):
        self.close_video()
        return super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = None
    try:
        node = VideoRecorder()
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    except Exception as error:
        print(f'video_recorder failed: {error}', file=sys.stderr)
        return 1
    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
    return 0


if __name__ == '__main__':
    sys.exit(main())
