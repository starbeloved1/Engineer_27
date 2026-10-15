#!/usr/bin/python3
"""Run against eng_bringup offline.launch.py in an isolated ROS_DOMAIN_ID using mock hardware."""
import json
import math
from pathlib import Path
import time
import unittest

import rclpy
from rclpy.action import ActionClient
from action_msgs.msg import GoalStatus
from eng_interfaces.action import PlanMotion
from moveit_msgs.msg import CollisionObject, PlanningScene, PlanningSceneComponents, RobotState
from moveit_msgs.srv import ApplyPlanningScene, GetPlanningScene, GetPositionFK, GetStateValidity
from shape_msgs.msg import SolidPrimitive
from sensor_msgs.msg import JointState

NAMES = [f'link_{i}_joint' for i in range(1, 7)]
RESULTS = {}


class OfflinePlanning(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()
        cls.node = rclpy.create_node('engineer27_integration_check')
        cls.action = ActionClient(cls.node, PlanMotion, '/arm_planning/plan_motion')
        if not cls.action.wait_for_server(timeout_sec=40.):
            raise RuntimeError('Start the offline demo in the same isolated ROS_DOMAIN_ID first')
        cls.validity = cls.node.create_client(GetStateValidity, '/check_state_validity')
        cls.fk = cls.node.create_client(GetPositionFK, '/compute_fk')
        cls.scene = cls.node.create_client(GetPlanningScene, '/get_planning_scene')
        cls.apply_scene = cls.node.create_client(ApplyPlanningScene, '/apply_planning_scene')
        for client in (cls.validity, cls.fk, cls.scene, cls.apply_scene):
            if not client.wait_for_service(timeout_sec=10.):
                raise RuntimeError('MoveIt services not ready')
        cls.joints = None
        def receive(message):
            cls.joints = dict(zip(message.name, message.position))
        cls.subscription = cls.node.create_subscription(JointState, '/joint_states', receive, 10)

    @classmethod
    def tearDownClass(cls):
        cls.node.destroy_node()
        rclpy.shutdown()

    def wait(self, future, timeout=30.):
        rclpy.spin_until_future_complete(self.node, future, timeout_sec=timeout)
        self.assertTrue(future.done(), 'ROS operation timed out')
        return future.result()

    def state(self, q):
        state = RobotState()
        state.joint_state.name = NAMES
        state.joint_state.position = list(q)
        state.is_diff = True
        return state

    def joint_goal(self, q, execute=False):
        goal = PlanMotion.Goal()
        goal.target_type = goal.JOINT
        goal.joint_positions = [float(x) for x in q]
        goal.execute = execute
        return goal

    def submit(self, goal, feedback=None):
        handle = self.wait(self.action.send_goal_async(goal, feedback_callback=feedback))
        self.assertTrue(handle.accepted)
        return handle, self.wait(handle.get_result_async(), 90.)

    def assert_valid_trajectory(self, result):
        self.assertTrue(result.success, result.message)
        self.assertEqual(result.error_code, 1)
        self.assertGreater(result.validation_samples, 0)
        trajectory = result.trajectory.joint_trajectory
        self.assertEqual(list(trajectory.joint_names), NAMES)
        self.assertGreater(len(trajectory.points), 1)
        previous_time = -1.
        for p in trajectory.points:
            stamp = p.time_from_start.sec + p.time_from_start.nanosec * 1e-9
            self.assertGreaterEqual(stamp, previous_time)
            previous_time = stamp
            self.assertTrue(all(math.isfinite(q) for q in p.positions))
            req = GetStateValidity.Request(robot_state=self.state(p.positions), group_name='arm_control')
            self.assertTrue(self.wait(self.validity.call_async(req)).valid)

    def test_01_known_collision_and_matrix(self):
        request = GetPlanningScene.Request()
        request.components.components = PlanningSceneComponents.ALLOWED_COLLISION_MATRIX
        matrix = self.wait(self.scene.call_async(request)).scene.allowed_collision_matrix
        for other in ('link_4', 'link_5'):
            i, j = matrix.entry_names.index('link_1'), matrix.entry_names.index(other)
            self.assertFalse(matrix.entry_values[i].enabled[j])
        request = GetStateValidity.Request(robot_state=self.state([0.] * 6), group_name='arm_control')
        self.assertTrue(self.wait(self.validity.call_async(request)).valid)
        request.robot_state = self.state([0., 0., .1, 0., 0., 0.])
        invalid = self.wait(self.validity.call_async(request))
        self.assertFalse(invalid.valid)
        pairs = sorted({tuple(sorted((c.contact_body_1, c.contact_body_2))) for c in invalid.contacts})
        self.assertIn(('link_1', 'link_4'), pairs)
        self.assertIn(('link_1', 'link_5'), pairs)
        RESULTS['known_collision_pairs'] = pairs

    def test_02_joint_planning_and_continuous_wrap(self):
        _, message = self.submit(self.joint_goal([2 * math.pi + .25, 0, 0, 0, 0, 0]))
        result = message.result
        self.assert_valid_trajectory(result)
        self.assertAlmostEqual(result.selected_joint_positions[0], .25, places=5)
        self.assertLess(result.joint_path_length, .5)
        RESULTS['joint_plan'] = {'points': len(result.trajectory.joint_trajectory.points),
                                 'samples': result.validation_samples, 'path_length': result.joint_path_length}

    def test_03_collision_goal_is_rejected(self):
        _, message = self.submit(self.joint_goal([0, 0, .1, 0, 0, 0]))
        self.assertEqual(message.status, GoalStatus.STATUS_ABORTED)
        self.assertFalse(message.result.success)
        self.assertEqual(message.result.error_code, -12)
        self.assertIn('intersects', message.result.message)
        RESULTS['collision_goal'] = message.result.message

    def test_04_invalid_request_rejected(self):
        goal = self.joint_goal([0.] * 6)
        goal.joint_positions[0] = float('nan')
        self.assertFalse(self.wait(self.action.send_goal_async(goal)).accepted)
        goal = self.joint_goal([0.] * 6)
        goal.velocity_scaling = 2.
        self.assertFalse(self.wait(self.action.send_goal_async(goal)).accepted)

    def test_05_pose_ik_and_unreachable_goal(self):
        target = [.2, 0., 0., 0., .2, 0.]
        valid = self.wait(self.validity.call_async(GetStateValidity.Request(robot_state=self.state(target), group_name='arm_control')))
        self.assertTrue(valid.valid)
        request = GetPositionFK.Request()
        request.header.frame_id = 'base_link'
        request.fk_link_names = ['link_6']
        request.robot_state = self.state(target)
        fk = self.wait(self.fk.call_async(request))
        self.assertEqual(fk.error_code.val, 1)
        goal = PlanMotion.Goal()
        goal.target_type = goal.POSE
        goal.target_pose = fk.pose_stamped[0]
        _, message = self.submit(goal)
        self.assert_valid_trajectory(message.result)
        self.assertGreaterEqual(message.result.ik_candidates, 1)
        request.robot_state = self.state(message.result.selected_joint_positions)
        solved = self.wait(self.fk.call_async(request)).pose_stamped[0].pose
        desired = goal.target_pose.pose
        distance = math.sqrt(sum((getattr(solved.position, a) - getattr(desired.position, a))**2 for a in 'xyz'))
        dot = abs(sum(getattr(solved.orientation, a) * getattr(desired.orientation, a) for a in 'xyzw'))
        self.assertLess(distance, 1e-4)
        self.assertGreater(dot, 1 - 1e-5)
        RESULTS['pose_plan'] = {'ik_candidates': message.result.ik_candidates,
                               'position_error_m': distance, 'orientation_quaternion_dot': dot,
                               'pose': [desired.position.x, desired.position.y, desired.position.z,
                                        desired.orientation.x, desired.orientation.y, desired.orientation.z, desired.orientation.w]}
        goal.target_pose.pose.position.x = 10.
        _, failed = self.submit(goal)
        self.assertFalse(failed.result.success)
        self.assertEqual(failed.result.error_code, -31)

    def test_06_world_scene_obstacle(self):
        obstacle = CollisionObject()
        obstacle.id = 'offline_test_obstacle'
        obstacle.header.frame_id = 'base_link'
        obstacle.operation = obstacle.ADD
        primitive = SolidPrimitive(type=SolidPrimitive.BOX, dimensions=[1., 1., 1.])
        from geometry_msgs.msg import Pose
        pose = Pose()
        pose.orientation.w = 1.
        pose.position.z = .2
        obstacle.primitives = [primitive]
        obstacle.primitive_poses = [pose]
        scene = PlanningScene(is_diff=True)
        scene.world.collision_objects = [obstacle]
        try:
            self.assertTrue(self.wait(self.apply_scene.call_async(ApplyPlanningScene.Request(scene=scene))).success)
            _, failed = self.submit(self.joint_goal([.2, 0, 0, 0, 0, 0]))
            self.assertEqual(failed.result.error_code, -10)
            self.assertIn(obstacle.id, failed.result.message)
        finally:
            obstacle.operation = obstacle.REMOVE
            scene.world.collision_objects = [obstacle]
            self.assertTrue(self.wait(self.apply_scene.call_async(ApplyPlanningScene.Request(scene=scene))).success)
        RESULTS['world_obstacle_checked'] = True

    def test_07_mock_execution_and_cancellation(self):
        _, executed = self.submit(self.joint_goal([.2, 0, 0, 0, 0, 0], execute=True))
        self.assert_valid_trajectory(executed.result)
        for _ in range(5):
            rclpy.spin_once(self.node, timeout_sec=.05)
        self.assertAlmostEqual(self.joints['link_1_joint'], .2, delta=.01)
        stages = []
        # Stay inside the verified collision-free yaw range; move slowly enough
        # to exercise cancellation while the controller is executing.
        goal = self.joint_goal([.25, 0, 0, 0, 0, 0], execute=True)
        goal.velocity_scaling = .005
        handle = self.wait(self.action.send_goal_async(goal, feedback_callback=lambda m: stages.append(m.feedback.stage)))
        self.assertTrue(handle.accepted)
        deadline = time.monotonic() + 20
        while 'executing' not in stages and time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=.05)
        self.assertIn('executing', stages)
        duplicate = self.wait(self.action.send_goal_async(self.joint_goal([0.] * 6)))
        self.assertFalse(duplicate.accepted)
        canceled = self.wait(handle.cancel_goal_async())
        self.assertTrue(canceled.goals_canceling)
        result = self.wait(handle.get_result_async(), 10.)
        self.assertEqual(result.status, GoalStatus.STATUS_CANCELED)
        self.assertEqual(result.result.error_code, -7)
        _, restored = self.submit(self.joint_goal([0.] * 6, execute=True))
        self.assertTrue(restored.result.success, restored.result.message)
        RESULTS['mock_execution_and_cancellation'] = True

    def test_08_detour_around_self_collision(self):
        # Both endpoints are valid, but direct joint interpolation crosses the
        # known q3=0.1 interference. The planner must change other joints.
        for q3 in (0., 2.):
            request = GetStateValidity.Request(
                robot_state=self.state([0., 0., q3, 0., 0., 0.]), group_name='arm_control')
            self.assertTrue(self.wait(self.validity.call_async(request)).valid)
        request.robot_state = self.state([0., 0., .1, 0., 0., 0.])
        self.assertFalse(self.wait(self.validity.call_async(request)).valid)
        goal = self.joint_goal([0., 0., 2., 0., 0., 0.])
        goal.planning_time = 10.
        _, message = self.submit(goal)
        self.assert_valid_trajectory(message.result)
        self.assertGreater(message.result.joint_path_length, 2.)
        deviation = max(abs(p.positions[j]) for p in message.result.trajectory.joint_trajectory.points
                        for j in (0, 1, 3, 4, 5))
        self.assertGreater(deviation, .05)
        RESULTS['self_collision_detour'] = {
            'direct_interpolation_invalid': True,
            'points': len(message.result.trajectory.joint_trajectory.points),
            'validation_samples': message.result.validation_samples,
            'path_length': message.result.joint_path_length,
            'other_joint_deviation_rad': deviation,
        }


if __name__ == '__main__':
    program = unittest.main(exit=False, verbosity=2)
    results = Path(__file__).resolve().parents[4] / '.cache/planning_validation'
    results.mkdir(parents=True, exist_ok=True)
    RESULTS['tests_run'] = program.result.testsRun
    RESULTS['passed'] = program.result.wasSuccessful()
    (results / 'integration_results.json').write_text(json.dumps(RESULTS, indent=2) + '\n')
    raise SystemExit(0 if program.result.wasSuccessful() else 1)
