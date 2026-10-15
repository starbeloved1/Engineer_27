#!/usr/bin/python3
"""在独立 ROS 域的离线系统中验证安装后的 C++ 客户端"""
import json
from pathlib import Path
import selectors
import signal
import subprocess
import tempfile
import time
import unittest

from ament_index_python.packages import get_package_prefix
import rclpy
from sensor_msgs.msg import JointState


class ClientIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.executable = str(Path(get_package_prefix('arm_goal')) / 'lib/arm_goal/plan_goal')
        cls.directory = tempfile.TemporaryDirectory(prefix='engineer27_client_')
        cls.output = Path(cls.directory.name)
        rclpy.init()
        cls.node = rclpy.create_node('client_integration_check')
        cls.joints = None

        def receive(message):
            cls.joints = dict(zip(message.name, message.position))
        cls.subscription = cls.node.create_subscription(JointState, '/joint_states', receive, 10)

    @classmethod
    def tearDownClass(cls):
        cls.node.destroy_node()
        rclpy.shutdown()
        cls.directory.cleanup()

    def command(self, args, code=0):
        result = subprocess.run([self.executable, *args], text=True, capture_output=True, timeout=35)
        self.assertEqual(result.returncode, code, result.stdout + result.stderr)
        return result

    def positions(self):
        # 消费旧缓存后等待新的反馈
        for _ in range(5):
            rclpy.spin_once(self.node, timeout_sec=.05)
        self.assertIsNotNone(self.joints)
        return dict(self.joints)

    def test_01_native_binary_help_and_invalid_input(self):
        self.assertEqual(Path(self.executable).read_bytes()[:4], b'\x7fELF')
        self.assertIn('--joints', self.command(['--help']).stdout)
        self.command(['--joints', '0'], 2)
        self.command(['--joints', 'nan', '0', '0', '0', '0', '0'], 2)
        self.command(['--pose', '0', '0', '0', '0', '0', '0', '0'], 2)

    def test_02_yaml_and_cli_parameter_override_without_server(self):
        start = time.monotonic()
        result = self.command(['--joints', '0', '0', '0', '0', '0', '0', '--ros-args',
                               '-p', 'planning_action:=/missing/test_server', '-p', 'server_timeout:=0.2'], 2)
        self.assertIn('Planning server unavailable', result.stderr)
        self.assertLess(time.monotonic() - start, 4.)

    def test_03_joint_json_without_execution(self):
        before = self.positions()
        path = self.output / 'nested/joints.json'
        self.command(['--joints', '.25', '0', '0', '0', '0', '0', '--output', str(path)])
        data = json.loads(path.read_text())
        self.assertTrue(data['success'])
        self.assertEqual(len(data['joint_names']), 6)
        self.assertGreater(data['validation_samples'], 0)
        self.assertGreater(len(data['points']), 1)
        self.assertAlmostEqual(data['selected_joint_positions'][0], .25, delta=.001)
        for point in data['points']:
            self.assertEqual(len(point['positions']), 6)
            self.assertEqual(len(point['velocities']), 6)
            self.assertIn('accelerations', point)
            self.assertGreaterEqual(point['time_from_start'], 0)
        after = self.positions()
        for name in before:
            self.assertAlmostEqual(before[name], after[name], delta=.001)

    def test_04_pose_json_and_collision_error(self):
        path = self.output / 'pose.json'
        self.command(['--pose', '.0534446', '.00930325', '.19012849', '-.00996671',
                      '.09933467', '.09933467', '.99003329', '--output', str(path)])
        self.assertGreater(json.loads(path.read_text())['ik_candidates'], 0)
        self.command(['--joints', '0', '0', '.1', '0', '0', '0', '--output', str(path)], 1)
        self.assertEqual(json.loads(path.read_text())['error_code'], -12)

    def test_05_execute_busy_and_ctrl_c_cancel(self):
        self.command(['--joints', '.2', '0', '0', '0', '0', '0', '--execute'])
        self.assertAlmostEqual(self.positions()['link_1_joint'], .2, delta=.01)
        process = subprocess.Popen([self.executable, '--joints', '.25', '0', '0', '0', '0', '0',
                                    '--execute', '--velocity-scale', '.005'], stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT, text=True, bufsize=1)
        selector = selectors.DefaultSelector()
        selector.register(process.stdout, selectors.EVENT_READ)
        lines = []
        try:
            deadline = time.monotonic() + 15
            while time.monotonic() < deadline:
                if selector.select(timeout=.2):
                    line = process.stdout.readline()
                    lines.append(line)
                    if line.startswith('executing:'):
                        break
                self.assertIsNone(process.poll(), ''.join(lines))
            else:
                self.fail('Client never reached executing stage: ' + ''.join(lines))
            rejected = self.command(['--joints', '0', '0', '0', '0', '0', '0'], 2)
            self.assertIn('server busy', rejected.stderr)
            process.send_signal(signal.SIGINT)
            rest, _ = process.communicate(timeout=8)
            self.assertEqual(process.returncode, 130, ''.join(lines) + rest)
            time.sleep(.2)
            self.command(['--joints', '0', '0', '0', '0', '0', '0', '--execute'])
        finally:
            selector.close()
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
                process.communicate(timeout=8)

    def test_06_request_timeout_cancels_task(self):
        self.command(['--joints', '0', '0', '0', '0', '0', '0', '--execute'])
        # 整个请求含约 1 s 状态等待，随后以低速执行，确保结果等待超时
        result = self.command(['--joints', '.25', '0', '0', '0', '0', '0', '--execute',
                               '--velocity-scale', '.005', '--timeout', '2'], 2)
        self.assertIn('cancellation requested', result.stderr)
        time.sleep(.2)
        self.command(['--joints', '0', '0', '0', '0', '0', '0', '--execute'])


if __name__ == '__main__':
    unittest.main(verbosity=2)
