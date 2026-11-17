#!/usr/bin/env python3
"""完整 pick 的真实 ROS/GenericSystem 回归；不启动相机或串口。先 source 本工作区。"""
import os
import math
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[1]
os.environ['ROS_DOMAIN_ID'] = '214'
sys.path.insert(0, str(ROOT / 'ros2_ws/src/engineer_mtc/scripts'))
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, DurabilityPolicy
from std_srvs.srv import Trigger
from std_msgs.msg import Bool
from sensor_msgs.msg import JointState
from geometry_msgs.msg import PoseStamped
from visualization_msgs.msg import Marker
from moveit_msgs.msg import PlanningSceneComponents
from moveit_msgs.srv import GetPlanningScene
from controller_manager_msgs.srv import SwitchController, ListHardwareComponents
from engineer_mtc.msg import PickStatus
from demo_geometry import call
from demo_pick import simulation_target


class PickIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory(prefix='engineer_pick_')
        cls.log_path = Path(cls.temporary.name) / 'system.log'
        cls.log = cls.log_path.open('w')
        cls.process = subprocess.Popen([str(ROOT/'run.sh'), 'use_rviz:=false',
            'mtc_config:='+str(ROOT/'ros2_ws/src/engineer_mtc/config/pick_demo.yaml')],
            cwd=ROOT, stdout=cls.log, stderr=subprocess.STDOUT, start_new_session=True)
        rclpy.init()
        cls.node = Node('pick_integration')
        cls.statuses, cls.claws, cls.markers, cls.joints = [], [], [], []
        qos = QoSProfile(depth=20, durability=DurabilityPolicy.TRANSIENT_LOCAL)
        cls.node.create_subscription(PickStatus, '/engineer_mtc/pick_status', cls.statuses.append, qos)
        cls.node.create_subscription(Bool, '/gripper_close', lambda m: cls.claws.append(m.data), qos)
        cls.node.create_subscription(Marker, '/energy_display/marker', cls.markers.append, qos)
        cls.node.create_subscription(JointState, '/joint_states', cls.joints.append, 10)
        cls.publisher = cls.node.create_publisher(PoseStamped, '/engineer_mtc/manual_object_pose', 1)
        cls.target = None
        def publish():
            if cls.target is not None:
                cls.target.header.stamp = cls.node.get_clock().now().to_msg()
                cls.publisher.publish(cls.target)
        cls.timer = cls.node.create_timer(.08, publish)
        deadline = time.monotonic()+50
        while time.monotonic()<deadline:
            rclpy.spin_once(cls.node, timeout_sec=.1)
            if cls.process.poll() is not None:
                cls.cleanup()
                raise RuntimeError('模拟系统启动失败：'+cls.log_path.read_text()[-4000:])
            if cls.statuses and cls.joints:
                break
        else:
            cls.cleanup()
            raise RuntimeError('模拟系统未就绪')
        request = ListHardwareComponents.Request()
        hardware = call(cls.node, ListHardwareComponents, '/controller_manager/list_hardware_components', request)
        if [h.name for h in hardware.component] != ['Engineer27MockSystem']:
            cls.cleanup()
            raise RuntimeError('只允许测试自建 GenericSystem')

    @classmethod
    def cleanup(cls):
        if cls.process.poll() is None:
            cls.process.send_signal(signal.SIGINT)
            try: cls.process.wait(timeout=20)
            except subprocess.TimeoutExpired:
                os.killpg(cls.process.pid, signal.SIGKILL)
                cls.process.wait()
        cls.log.close()
        cls.node.destroy_node()
        if rclpy.ok(): rclpy.shutdown()

    @classmethod
    def tearDownClass(cls):
        cls.cleanup()
        output = cls.log_path.read_text()
        # 日志保留在 /tmp，便于失败时检查。
        Path('/tmp/engineer27_pick_integration.log').write_text(output)
        cls.temporary.cleanup()

    def spin(self, seconds):
        end = time.monotonic()+seconds
        while time.monotonic()<end:
            rclpy.spin_once(self.node, timeout_sec=.05)

    def wait(self, predicate, timeout=90):
        end = time.monotonic()+timeout
        while time.monotonic()<end:
            self.assertIsNone(self.process.poll(), self.log_path.read_text()[-3000:])
            rclpy.spin_once(self.node, timeout_sec=.05)
            if predicate(): return
        self.fail('等待超时；最近状态：'+str(self.statuses[-1])+'\n'+self.log_path.read_text()[-2000:])

    def trigger(self, name):
        return call(self.node, Trigger, '/engineer_mtc/'+name, Trigger.Request(), timeout=190)

    def scene(self):
        request = GetPlanningScene.Request()
        request.components.components = (PlanningSceneComponents.WORLD_OBJECT_GEOMETRY |
                                          PlanningSceneComponents.ROBOT_STATE_ATTACHED_OBJECTS)
        return call(self.node, GetPlanningScene, '/get_planning_scene', request).scene

    def planned(self, mode):
        type(self).target = simulation_target(self.node, mode)
        self.spin(.3)
        response = self.trigger('plan_manual')
        self.assertTrue(response.success, response.message)
        self.wait(lambda: self.statuses[-1].phase == PickStatus.READY)
        return self.statuses[-1].task_id

    def terminal(self, task_id, stage, phase):
        self.wait(lambda: self.statuses[-1].task_id == task_id and
            self.statuses[-1].stage == stage and self.statuses[-1].phase in
            [PickStatus.WAITING_NEXT, PickStatus.SUCCEEDED, PickStatus.FAILED, PickStatus.CANCELED])
        self.assertEqual(self.statuses[-1].phase, phase, self.statuses[-1].message)

    def test_01_missing_invalid_pose_never_creates_default_energy(self):
        self.assertFalse(self.trigger('plan').success)
        self.assertFalse(self.trigger('execute').success)
        bad = PoseStamped(); bad.header.frame_id = 'missing_camera_frame'; bad.pose.orientation.w = 1.
        type(self).target = bad
        self.spin(.3)
        self.assertFalse(self.trigger('plan_manual').success)
        self.spin(.3)
        self.assertFalse(any(o.id == 'energy' for o in self.scene().world.collision_objects))
        self.assertEqual(self.markers[-1].action, Marker.DELETE)
        type(self).target = None

    def test_02_step_attach_extract_release_and_home(self):
        task_id = self.planned('step')
        self.spin(.3)
        self.assertEqual(self.markers[-1].action, Marker.ADD)
        self.assertTrue(self.trigger('execute').success)
        for stage in range(1,7):
            self.terminal(task_id, stage, PickStatus.SUCCEEDED if stage==6 else PickStatus.WAITING_NEXT)
            actual = self.scene()
            attached = any(a.object.id=='energy' for a in actual.robot_state.attached_collision_objects)
            self.assertEqual(attached, 2<=stage<=4)
            self.assertEqual(self.statuses[-1].object_attached, attached)
            self.assertEqual(self.statuses[-1].gripper_closed, stage in [2,3,4,6])
            if stage==2:
                before = list(self.joints[-1].position)
                type(self).target = None  # 视觉丢失不删除持物，也不自己启动拉取。
                self.spin(1.7)
                self.assertEqual(self.statuses[-1].stage, 2)
                self.assertEqual(before, list(self.joints[-1].position))
                self.assertEqual(self.markers[-1].header.frame_id, 'link_6')
                self.assertTrue(self.markers[-1].frame_locked)
            if stage<6:
                self.assertTrue(self.trigger('next_stage').success)
        position = dict(zip(self.joints[-1].name,self.joints[-1].position))
        self.assertTrue(all(abs(math.atan2(math.sin(position[f'link_{i}_joint']),
            math.cos(position[f'link_{i}_joint'])))<.01 for i in range(1,7)), position)
        self.assertEqual(self.claws[-4:],[False,True,False,True])
        self.assertEqual(len(self.scene().world.collision_objects[0].primitives),3)
        self.assertFalse(self.trigger('next_stage').success)

    def test_03_auto_completes_without_next_commands(self):
        task_id = self.planned('auto')
        self.assertTrue(self.trigger('execute').success)
        self.terminal(task_id,6,PickStatus.SUCCEEDED)
        messages = [m for m in self.statuses if m.task_id==task_id]
        self.assertEqual({m.stage for m in messages if m.phase==PickStatus.EXECUTING},set(range(1,7)))
        self.assertFalse(any(m.phase==PickStatus.WAITING_NEXT for m in messages))
        self.assertFalse(self.scene().robot_state.attached_collision_objects)

    def test_04_failed_plan_invalidates_previous_solution(self):
        # 规划就绪后取消也必须作废旧解，不能在 execute 时重新复活。
        self.planned('auto')
        self.assertTrue(self.trigger('cancel').success)
        self.wait(lambda: self.statuses[-1].phase == PickStatus.CANCELED)
        self.assertFalse(self.trigger('execute').success)
        self.planned('auto')
        bad = PoseStamped();bad.header.frame_id='base_link';bad.pose.orientation.w=2.
        type(self).target=bad
        self.spin(.3)
        self.assertFalse(self.trigger('plan_manual').success)
        self.assertFalse(self.trigger('execute').success)
        type(self).target=None
        self.spin(.4)
        self.assertFalse(any(o.id=='energy' for o in self.scene().world.collision_objects))

    def test_05_controller_failure_does_not_advance(self):
        task_id = self.planned('auto')
        def switch(activate):
            request=SwitchController.Request();request.strictness=2;request.timeout.sec=5
            if activate:request.activate_controllers=['arm_controller']
            else:request.deactivate_controllers=['arm_controller']
            return call(self.node,SwitchController,'/controller_manager/switch_controller',request).ok
        self.assertTrue(switch(False))
        try:
            self.assertTrue(self.trigger('execute').success)
            self.terminal(task_id,1,PickStatus.FAILED)
            self.assertFalse(any(m.task_id==task_id and m.stage>1 for m in self.statuses))
        finally:
            self.assertTrue(switch(True))

    def test_06_cancel_extraction_preserves_grasp(self):
        task_id=self.planned('auto')
        self.assertTrue(self.trigger('execute').success)
        self.wait(lambda:self.statuses[-1].task_id==task_id and self.statuses[-1].stage==3 and
                  self.statuses[-1].phase==PickStatus.EXECUTING)
        self.assertTrue(self.trigger('cancel').success)
        self.terminal(task_id,3,PickStatus.CANCELED)
        self.spin(.5)
        self.assertTrue(self.statuses[-1].object_attached)
        self.assertTrue(self.claws[-1])
        self.assertTrue(self.scene().robot_state.attached_collision_objects)
        self.assertFalse(any(m.task_id==task_id and m.stage>3 for m in self.statuses))
        self.assertFalse(self.trigger('next_stage').success)
        self.assertFalse(self.trigger('execute').success)


if __name__=='__main__':
    unittest.main(verbosity=2)
