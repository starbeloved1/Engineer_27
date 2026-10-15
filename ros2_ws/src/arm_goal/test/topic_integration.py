#!/usr/bin/python3
"""在同域 offline 系统中验证 C++ Topic→Action 客户端，勿与其他运动测试并行。"""
from contextlib import contextmanager
import json
from pathlib import Path
import signal
import subprocess
import tempfile
import time
import unittest
import uuid

from ament_index_python.packages import get_package_prefix
import rclpy
from eng_interfaces.msg import MotionTarget, MotionFeedback, MotionResult
from eng_interfaces.action import PlanMotion
from moveit_msgs.msg import MoveItErrorCodes as Codes
from sensor_msgs.msg import JointState
from std_msgs.msg import String


class Channel:
    def __init__(self, node, prefix):
        self.publisher = node.create_publisher(MotionTarget, prefix + '/target', 10)
        self.cancel = node.create_publisher(String, prefix + '/cancel', 10)
        self.results = {}
        self.feedback = {}
        self.subscriptions = [
            node.create_subscription(MotionResult, prefix + '/result', self.result, 10),
            node.create_subscription(MotionFeedback, prefix + '/feedback', self.progress, 10)]

    def result(self, message):
        self.results.setdefault(message.request_id, []).append(message)

    def progress(self, message):
        self.feedback.setdefault(message.request_id, []).append(message.stage)


class TopicIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()
        cls.node = rclpy.create_node('topic_client_integration')
        cls.channel = Channel(cls.node, '/arm_goal')
        cls.executable = str(Path(get_package_prefix('arm_goal')) / 'lib/arm_goal/plan_goal')
        cls.joints = None
        def receive(message):
            cls.joints = dict(zip(message.name, message.position))
        cls.subscription = cls.node.create_subscription(JointState, '/joint_states', receive, 10)

    @classmethod
    def tearDownClass(cls):
        cls.node.destroy_node()
        rclpy.shutdown()

    def wait(self, condition, timeout=25.):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=.05)
            if condition():
                return
        self.fail('Timed out waiting for Topic event')

    def target(self, q=None, **kwargs):
        message = MotionTarget()
        message.request_id = uuid.uuid4().hex
        message.joint_positions = q or [0.] * 6
        for key, value in kwargs.items():
            setattr(message, key, value)
        return message

    def send(self, message, channel=None):
        channel = channel or self.channel
        self.wait(lambda: channel.publisher.get_subscription_count() > 0, 15.)
        channel.publisher.publish(message)

    def result(self, message, channel=None):
        channel = channel or self.channel
        self.wait(lambda: bool(channel.results.get(message.request_id)))
        return channel.results[message.request_id][-1]

    def positions(self):
        for _ in range(10):
            rclpy.spin_once(self.node, timeout_sec=.05)
        self.assertIsNotNone(self.joints)
        return dict(self.joints)

    def home(self):
        message = self.target(execute=True)
        self.send(message)
        result = self.result(message)
        self.assertTrue(result.success, result.message)

    @contextmanager
    def bridge(self, **overrides):
        prefix = '/test_goal_' + uuid.uuid4().hex[:8]
        channel = Channel(self.node, prefix)
        parameters = {'target_topic': prefix + '/target', 'cancel_topic': prefix + '/cancel',
                      'feedback_topic': prefix + '/feedback', 'result_topic': prefix + '/result',
                      'planning_action': '/arm_planning/plan_motion', 'server_timeout': 1.,
                      'acceptance_timeout': 3., 'request_timeout': 120.}
        parameters.update(overrides)
        with tempfile.TemporaryDirectory() as directory, tempfile.TemporaryFile(mode='w+') as log:
            config = Path(directory) / 'bridge.yaml'
            config.write_text(json.dumps({'test_goal_gateway': {'ros__parameters': parameters}}))
            process = subprocess.Popen([self.executable, '--listen', '--ros-args',
                                        '-r', '__node:=test_goal_gateway', '--params-file', str(config)],
                                       stdout=log, stderr=subprocess.STDOUT)
            try:
                self.wait(lambda: channel.publisher.get_subscription_count() > 0, 15.)
                yield channel
            finally:
                process.send_signal(signal.SIGINT)
                try:
                    code = process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                    self.fail('Topic client failed to stop')
                log.seek(0)
                self.assertEqual(code, 0, log.read())

    def test_00_topic_schemas_match_action_fields(self):
        for topic_type, action_type in [(MotionTarget, PlanMotion.Goal),
                                        (MotionFeedback, PlanMotion.Feedback),
                                        (MotionResult, PlanMotion.Result)]:
            fields = topic_type.get_fields_and_field_types()
            self.assertEqual(fields.pop('request_id'), 'string')
            self.assertEqual(fields, action_type.get_fields_and_field_types())

    def test_01_joint_preview_feedback_and_no_state_change(self):
        self.home()
        before = self.positions()
        message = self.target([.25, 0., 0., 0., 0., 0.])
        self.send(message)
        result = self.result(message)
        self.assertTrue(result.success, result.message)
        self.assertGreater(len(result.trajectory.joint_trajectory.points), 1)
        self.assertGreater(result.validation_samples, 0)
        self.assertAlmostEqual(result.selected_joint_positions[0], .25, delta=.001)
        # 接收响应与首条 Action 反馈可能竞争；不要求每个瞬时阶段都被观察到。
        self.wait(lambda: {'accepted', 'planning', 'validating'}.issubset(
            self.channel.feedback.get(message.request_id, [])))
        after = self.positions()
        for name in before:
            self.assertAlmostEqual(before[name], after[name], delta=.001)

    def test_02_pose_and_known_collision(self):
        message = self.target(target_type=MotionTarget.POSE)
        message.target_pose.header.frame_id = 'base_link'
        p = message.target_pose.pose.position
        p.x, p.y, p.z = .0534446, .00930325, .19012849
        q = message.target_pose.pose.orientation
        q.x, q.y, q.z, q.w = -.00996671, .09933467, .09933467, .99003329
        self.send(message)
        result = self.result(message)
        self.assertTrue(result.success, result.message)
        self.assertGreater(result.ik_candidates, 0)
        message = self.target([0., 0., .1, 0., 0., 0.])
        self.send(message)
        result = self.result(message)
        self.assertFalse(result.success)
        self.assertEqual(result.error_code, Codes.GOAL_IN_COLLISION)

    def test_03_invalid_input_does_not_stop_listener(self):
        message = self.target(request_id='')
        self.send(message)
        self.assertEqual(self.result(message).error_code, Codes.INVALID_GOAL_CONSTRAINTS)
        message = self.target(target_type=255)
        self.send(message)
        self.assertFalse(self.result(message).success)
        message = self.target()
        self.send(message)
        self.assertTrue(self.result(message).success)

    def test_04_execute_busy_duplicate_and_matching_cancel(self):
        self.home()
        message = self.target([.25, 0., 0., 0., 0., 0.], execute=True, velocity_scaling=.005)
        self.send(message)
        self.wait(lambda: 'executing' in self.channel.feedback.get(message.request_id, []))
        self.send(message)
        self.wait(lambda: 'duplicate_ignored' in self.channel.feedback.get(message.request_id, []))
        other = self.target()
        self.send(other)
        self.assertIn('busy', self.result(other).message)
        wrong = String(data='not_the_active_id')
        self.channel.cancel.publish(wrong)
        self.wait(lambda: 'cancel_ignored' in self.channel.feedback.get(wrong.data, []))
        self.channel.cancel.publish(String(data=message.request_id))
        result = self.result(message)
        self.assertFalse(result.success)
        self.assertEqual(result.error_code, Codes.PREEMPTED)
        self.assertEqual(len(self.channel.results[message.request_id]), 1)
        self.home()

    def test_05_execution_changes_feedback_state(self):
        message = self.target([.2, 0., 0., 0., 0., 0.], execute=True)
        self.send(message)
        result = self.result(message)
        self.assertTrue(result.success, result.message)
        self.assertAlmostEqual(self.positions()['link_1_joint'], .2, delta=.01)
        self.home()

    def test_06_yaml_custom_topics_and_missing_action_server(self):
        with self.bridge(planning_action='/missing/topic_test', server_timeout=.2) as channel:
            for _ in range(2):
                message = self.target()
                self.send(message, channel)
                result = self.result(message, channel)
                self.assertFalse(result.success)
                self.assertEqual(result.error_code, Codes.COMMUNICATION_FAILURE)
                self.assertIn('unavailable', result.message)

    def test_07_request_timeout_cancels_execution(self):
        with self.bridge(request_timeout=2.) as channel:
            message = self.target([.25, 0., 0., 0., 0., 0.], execute=True, velocity_scaling=.005)
            self.send(message, channel)
            result = self.result(message, channel)
            self.assertFalse(result.success)
            self.assertEqual(result.error_code, Codes.TIMED_OUT)
            self.assertIn('cancellation requested', result.message)
        self.home()


if __name__ == '__main__':
    unittest.main(verbosity=2)
