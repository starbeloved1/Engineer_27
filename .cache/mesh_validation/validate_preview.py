"""Isolated RViz render and eight-pose TF validation; does not use the user's ROS graph."""
import json
import math
import os
from pathlib import Path
import signal
import subprocess
import sys
import time
import xml.etree.ElementTree as ET

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import merge_link_meshes as merger

output = Path(__file__).parent
mode = os.environ.get('PREVIEW_MODE', 'visual')
parent_display = os.environ.get('DISPLAY', ':1')
display = next(f':{i}' for i in range(124, 140) if not Path(f'/tmp/.X11-unix/X{i}').exists())
env = dict(os.environ, DISPLAY=display, LIBGL_ALWAYS_SOFTWARE='1', ROS_DOMAIN_ID='185', QT_QPA_PLATFORM='xcb')
processes = []
node = None
try:
    screen_log = (output / 'xephyr.log').open('w')
    xephyr = subprocess.Popen(['Xephyr', display, '-screen', '1280x800', '-nolisten', 'tcp', '-noreset', '-ac'],
                              env=dict(os.environ, DISPLAY=parent_display), stdout=screen_log, stderr=subprocess.STDOUT,
                              start_new_session=True)
    processes.append(xephyr)
    for _ in range(100):
        if Path('/tmp/.X11-unix/X' + display[1:]).exists():
            break
        if xephyr.poll() is not None:
            raise RuntimeError('Xephyr failed; inspect xephyr.log')
        time.sleep(.05)
    os.environ.update(env)
    from PyQt5.QtWidgets import QApplication
    from PyQt5.QtCore import Qt
    import rclpy
    from sensor_msgs.msg import JointState
    from tf2_msgs.msg import TFMessage

    app = QApplication([])
    rclpy.init()
    node = rclpy.create_node('light_visual_validation')
    publisher = node.create_publisher(JointState, '/joint_states', 10)
    latest = {}
    def receive(message):
        for transform in message.transforms:
            latest[transform.child_frame_id] = transform
    subscription = node.create_subscription(TFMessage, '/tf', receive, 100)
    joints = ET.parse(ROOT / 'assets' / 'arm.urdf').getroot().findall('joint')
    names = [j.get('name') for j in joints]
    q = [0.] * len(names)
    def tick():
        message = JointState()
        message.header.stamp = node.get_clock().now().to_msg()
        message.name = names
        message.position = q
        publisher.publish(message)
        rclpy.spin_once(node, timeout_sec=.03)
        app.processEvents()
    def capture():
        shot = app.primaryScreen().grabWindow(0)
        image = shot.toImage().convertToFormat(4)  # ARGB32: little-endian B,G,R,A
        ptr = image.bits()
        ptr.setsize(image.byteCount())
        pixels = np.frombuffer(ptr, dtype=np.uint8).reshape(image.height(), image.bytesPerLine() // 4, 4)[:, :image.width()]
        rgb = pixels[:, :, [2, 1, 0]].astype(np.int16)
        red = (rgb[:, :, 0] > 90) & (rgb[:, :, 0] > rgb[:, :, 1] * 1.4) & (rgb[:, :, 0] > rgb[:, :, 2] * 1.4)
        return shot, int(red.sum())
    runtime_log = (output / f'runtime_{mode}.log').open('w')
    started = time.monotonic()
    command = ['ros2', 'launch', str(ROOT / 'assets' / 'preview.launch.py'), 'joint_states:=false']
    if mode == 'collision':
        command.append('rviz_config:=' + str(ROOT / 'assets/preview_collision.rviz'))
    launch = subprocess.Popen(command,
                              env=env, stdout=runtime_log, stderr=subprocess.STDOUT, start_new_session=True)
    processes.append(launch)
    red_pixels = 0
    while time.monotonic() - started < 90:
        tick()
        shot, red_pixels = capture()
        if red_pixels > 1000 and len(latest) >= 6:
            break
        if launch.poll() is not None:
            raise RuntimeError('Preview exited before rendering')
    if red_pixels <= 1000:
        capture()[0].save(str(output / 'failed_preview.png'))
        raise RuntimeError('No model rendered within 90 seconds')
    load_seconds = time.monotonic() - started
    for _ in range(30):
        tick()
    capture()[0].save(str(output / f'{mode}_zero.png'))

    def expected(joint, angle):
        origin = joint.find('origin')
        R = merger.rotation(merger.vector(origin.get('rpy') if origin is not None else None, '0 0 0'))
        xyz = merger.vector(origin.get('xyz') if origin is not None else None, '0 0 0')
        axis = merger.vector(joint.find('axis').get('xyz'), '1 0 0')
        axis = axis / np.linalg.norm(axis)
        x, y, z = axis
        K = np.array([[0, -z, y], [z, 0, -x], [-y, x, 0]])
        return xyz, R @ (np.eye(3) + math.sin(angle) * K + (1 - math.cos(angle)) * (K @ K))

    poses = [[0.] * 6] + [[.1 if j == i else 0. for j in range(6)] for i in range(6)] + [[.2, .3, -.4, .5, .6, .7]]
    max_error = 0.
    for pose in poses:
        q[:] = pose
        latest.clear()
        pose_start = node.get_clock().now().nanoseconds
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            tick()
            fresh = {key: t for key, t in latest.items()
                     if t.header.stamp.sec * 10**9 + t.header.stamp.nanosec >= pose_start}
            if all(j.find('child').get('link') in fresh for j in joints):
                break
        for joint, angle in zip(joints, pose):
            actual = fresh[joint.find('child').get('link')]
            if actual.header.frame_id != joint.find('parent').get('link'):
                raise RuntimeError('TF parent changed')
            translation, R = expected(joint, angle)
            t, r = actual.transform.translation, actual.transform.rotation
            x, y, z, w = r.x, r.y, r.z, r.w
            actual_R = np.array([[1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
                                 [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
                                 [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)]])
            max_error = max(max_error, float(np.max(np.abs(translation - [t.x, t.y, t.z]))),
                            float(np.max(np.abs(R - actual_R))))
        for _ in range(12):
            tick()
    capture()[0].save(str(output / f'{mode}_combined.png'))
    if max_error > 1e-10:
        raise RuntimeError(f'TF error: {max_error}')
    result = {'time_to_first_model_seconds': load_seconds, 'software_opengl': True,
              'red_model_pixels': red_pixels, 'checked_poses': len(poses), 'max_tf_matrix_error': max_error}
    (output / f'preview_validation_{mode}.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2), flush=True)
finally:
    if node is not None:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
    for process in reversed(processes):
        if process.poll() is None:
            os.killpg(process.pid, signal.SIGINT)
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
