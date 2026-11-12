"""验证相机图像经 main_visualizer 和 Bridge 到达 Foxglove 协议客户端"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import socket
import struct
import subprocess
import tempfile
import time

import rclpy
from geometry_msgs.msg import TransformStamped
from rclpy.qos import QoSProfile, ReliabilityPolicy
from rclpy.serialization import deserialize_message
from sensor_msgs.msg import CameraInfo, Image, PointCloud2, PointField
from tf2_msgs.msg import TFMessage
from tf2_ros import StaticTransformBroadcaster
import websocket


ROOT = Path(__file__).resolve().parents[4]
COLOR_TOPIC = "/camera/camera/color/image_raw"
VISUALIZATION_TOPIC = "/vision/visualization/image"
CLOUD_TOPIC = "/camera/camera/depth/color/points"


def descendants(pid):
    try:
        children = Path(f"/proc/{pid}/task/{pid}/children").read_text().split()
    except FileNotFoundError:
        return set()
    result = {int(child) for child in children}
    for child in tuple(result):
        result.update(descendants(child))
    return result


def running(pid):
    try:
        return Path(f"/proc/{pid}/stat").read_text().rsplit(")", 1)[1].split()[0] != "Z"
    except FileNotFoundError:
        return False


def image_key(image):
    return (image.header.stamp.sec, image.header.stamp.nanosec, image.header.frame_id)


def image_content(image):
    return (image.encoding, image.height, image.width, image.step, image.is_bigendian,
            hashlib.sha256(image.data).digest())


def run(camera=False):
    previous_domain = os.environ.get("ROS_DOMAIN_ID")
    os.environ["ROS_DOMAIN_ID"] = "193"
    # 测试独占临时端口，不占用用户显示端的 8765
    with socket.socket() as port_socket:
        port_socket.bind(("127.0.0.1", 0))
        port = port_socket.getsockname()[1]
    with tempfile.TemporaryDirectory(prefix="engineer_visualization_") as directory:
        log_path = Path(directory) / "launch.log"
        config = Path(directory) / "bridge.yaml"
        default = ROOT / "ros2_ws/src/vision_nodes/config/foxglove_bridge.yaml"
        config.write_text(default.read_text().replace("port: 8765", f"port: {port}"))
        command = [str(ROOT / "run.sh"), "mode:=vision",
                   f"start_camera:={str(camera).lower()}", f"foxglove_config:={config}"]
        rclpy.init()
        node = rclpy.create_node("visualization_pipeline_check")
        qos = QoSProfile(depth=1, reliability=ReliabilityPolicy.RELIABLE)
        color_pub = None if camera else node.create_publisher(Image, COLOR_TOPIC, qos)
        cloud_pub = None if camera else node.create_publisher(PointCloud2, CLOUD_TOPIC, qos)
        broadcaster = None if camera else StaticTransformBroadcaster(node)
        source = Image(height=480, width=640, encoding="rgb8", step=640 * 3)
        source.header.frame_id = "camera_color_optical_frame"
        source.data = bytes([25, 100, 200]) * (640 * 480)
        cloud = PointCloud2(height=1, width=1, is_dense=True, point_step=16, row_step=16)
        cloud.header.frame_id = "camera_color_optical_frame"
        cloud.fields = [PointField(name=name, offset=offset, datatype=PointField.FLOAT32, count=1)
                        for name, offset in [("x", 0), ("y", 4), ("z", 8), ("rgb", 12)]]
        cloud.data = struct.pack("<fffI", 0.0, 0.0, 0.5, 0xFF0000)
        if broadcaster:
            transform = TransformStamped()
            transform.header.stamp = node.get_clock().now().to_msg()
            transform.header.frame_id = "camera_link"
            transform.child_frame_id = source.header.frame_id
            transform.transform.rotation.w = 1.0
            broadcaster.sendTransform(transform)
        topics = {COLOR_TOPIC: Image, VISUALIZATION_TOPIC: Image,
                  CLOUD_TOPIC: PointCloud2, "/tf_static": TFMessage}
        if camera:
            topics.update({"/camera/camera/aligned_depth_to_color/image_raw": Image,
                           "/camera/camera/color/camera_info": CameraInfo})
        with log_path.open("w") as log:
            process = subprocess.Popen(command, cwd=ROOT, stdout=log,
                                       stderr=subprocess.STDOUT, start_new_session=True)
            client = None
            tracked = set()
            try:
                channels = {}
                subscriptions = {}
                counts = {}
                originals, visualizations = {}, {}
                next_publish = 0.0
                last_messages = {}
                deadline = time.monotonic() + 35
                while time.monotonic() < deadline:
                    assert process.poll() is None, log_path.read_text()
                    tracked.update(descendants(process.pid))
                    if color_pub is not None and time.monotonic() >= next_publish:
                        source.header.stamp = node.get_clock().now().to_msg()
                        cloud.header.stamp = source.header.stamp
                        color_pub.publish(source)
                        cloud_pub.publish(cloud)
                        next_publish = time.monotonic() + 1.0 / 6.0
                    rclpy.spin_once(node, timeout_sec=0.01)
                    if client is None:
                        try:
                            client = websocket.create_connection(
                                f"ws://127.0.0.1:{port}", timeout=2.0,
                                subprotocols=["foxglove.sdk.v1"],
                                http_no_proxy=["127.0.0.1", "localhost"])
                            client.settimeout(0.05)
                        except (ConnectionRefusedError, websocket.WebSocketTimeoutException):
                            continue
                    try:
                        message = client.recv()
                    except websocket.WebSocketTimeoutException:
                        continue
                    if isinstance(message, str):
                        event = json.loads(message)
                        if event["op"] == "advertise":
                            channels.update({channel["topic"]: channel
                                             for channel in event["channels"]})
                        for topic in topics:
                            if topic in channels and topic not in subscriptions.values():
                                channel = channels[topic]
                                assert channel["encoding"] == "cdr"
                                identifier = len(subscriptions) + 1
                                subscriptions[identifier] = topic
                                client.send(json.dumps({"op": "subscribe", "subscriptions": [
                                    {"id": identifier, "channelId": channel["id"]}]}))
                        continue
                    if not message or message[0] != 1:
                        continue
                    identifier = struct.unpack_from("<I", message, 1)[0]
                    topic = subscriptions[identifier]
                    decoded = deserialize_message(message[13:], topics[topic])
                    counts[topic] = counts.get(topic, 0) + 1
                    last_messages[topic] = decoded
                    if topic == COLOR_TOPIC:
                        originals[image_key(decoded)] = image_content(decoded)
                    elif topic == VISUALIZATION_TOPIC:
                        visualizations[image_key(decoded)] = image_content(decoded)
                    matched = originals.keys() & visualizations.keys()
                    if (all(topic in last_messages for topic in topics)
                            and len(matched) >= 12):
                        assert all(originals[key] == visualizations[key] for key in matched)
                        assert last_messages[CLOUD_TOPIC].width > 0
                        assert "rgb" in [field.name for field in last_messages[CLOUD_TOPIC].fields]
                        assert last_messages["/tf_static"].transforms
                        if camera:
                            assert last_messages["/camera/camera/color/camera_info"].k[0] > 0
                        names = node.get_node_names()
                        assert "main_visualizer" in names
                        assert "depth_preview" not in names and "vision_rviz" not in names
                        break
                else:
                    raise AssertionError(f"Foxglove 数据不完整: {counts}")
                client.close()
                client = None
                rclpy.spin_once(node, timeout_sec=0.1)
                assert process.poll() is None, "关闭显示端不应关闭视觉流水线"
                process.send_signal(signal.SIGINT)
                assert process.wait(timeout=20) == 0, log_path.read_text()
                assert not [pid for pid in tracked if running(pid)], log_path.read_text()
                assert "Segmentation fault" not in log_path.read_text()
                print(("D435" if camera else "Synthetic") + " Foxglove integration PASS")
                print("WebSocket CDR topics:", counts)
                print("12 matching frames preserved timestamp, frame, encoding and pixels")
            except Exception:
                print(log_path.read_text()[-8000:])
                raise
            finally:
                if client is not None:
                    client.close()
                if process.poll() is None:
                    process.send_signal(signal.SIGINT)
                    try:
                        process.wait(timeout=20)
                    except subprocess.TimeoutExpired:
                        for pid in descendants(process.pid):
                            try:
                                os.kill(pid, signal.SIGKILL)
                            except ProcessLookupError:
                                pass
                        process.kill()
                        process.wait()
                node.destroy_node()
                rclpy.shutdown()
                if previous_domain is None:
                    os.environ.pop("ROS_DOMAIN_ID", None)
                else:
                    os.environ["ROS_DOMAIN_ID"] = previous_domain


def test_visualization_pipeline():
    run()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--camera", action="store_true", help="使用真实 D435，先关闭占用相机的程序")
    run(camera=parser.parse_args().camera)
