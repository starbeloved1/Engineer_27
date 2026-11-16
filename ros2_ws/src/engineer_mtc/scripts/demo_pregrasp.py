#!/usr/bin/python3
"""从一个已知关节目标的 FK 反推虚拟矿位姿，再触发真实 MTC 规划。"""
import math
import time

import numpy as np
import rclpy
from geometry_msgs.msg import PoseStamped
from moveit_msgs.srv import GetPositionFK
from rcl_interfaces.srv import GetParameters
from std_srvs.srv import Trigger


def xyz_rpy(xyz, rpy):
    roll, pitch, yaw = rpy
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    result = np.eye(4)
    result[:3, :3] = [[cy*cp, cy*sp*sr-sy*cr, cy*sp*cr+sy*sr],
                      [sy*cp, sy*sp*sr+cy*cr, sy*sp*cr-cy*sr], [-sp, cp*sr, cp*cr]]
    result[:3, 3] = xyz
    return result


def pose_matrix(pose):
    x, y, z, w = pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w
    result = np.eye(4)
    result[:3, :3] = [[1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
                      [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
                      [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)]]
    result[:3, 3] = [pose.position.x, pose.position.y, pose.position.z]
    return result


def matrix_pose(matrix):
    message = PoseStamped()
    message.pose.position.x, message.pose.position.y, message.pose.position.z = map(float, matrix[:3, 3])
    r = matrix[:3, :3]
    # 四种分支避免接近 180 度旋转时用 trace 公式产生数值奇异。
    if np.trace(r) > 0:
        s = math.sqrt(1 + np.trace(r)) * 2
        q = [(r[2, 1]-r[1, 2])/s, (r[0, 2]-r[2, 0])/s, (r[1, 0]-r[0, 1])/s, s/4]
    else:
        i = int(np.argmax(np.diag(r)))
        j, k = (i+1) % 3, (i+2) % 3
        s = math.sqrt(1+r[i, i]-r[j, j]-r[k, k]) * 2
        q = [0., 0., 0., (r[k, j]-r[j, k])/s]
        q[i], q[j], q[k] = s/4, (r[j, i]+r[i, j])/s, (r[k, i]+r[i, k])/s
    message.pose.orientation.x, message.pose.orientation.y, message.pose.orientation.z, \
        message.pose.orientation.w = map(float, q)
    return message


def call(node, kind, name, request, timeout=35):
    client = node.create_client(kind, name)
    try:
        if not client.wait_for_service(timeout_sec=10):
            raise RuntimeError(f'服务未就绪：{name}，请先运行 ./run.sh mode:=mtc')
        future = client.call_async(request)
        rclpy.spin_until_future_complete(node, future, timeout_sec=timeout)
        if not future.done() or future.result() is None:
            raise RuntimeError(f'服务超时或失败：{name}')
        return future.result()
    finally:
        node.destroy_client(client)


def demo_target(node):
    names = ['base_frame', 'tip_link', 'object_to_tcp_xyz', 'object_to_tcp_rpy',
             'tip_to_tcp_xyz', 'tip_to_tcp_rpy', 'approach_axis', 'pregrasp_distance']
    request = GetParameters.Request()
    request.names = names
    values = call(node, GetParameters, '/engineer_mtc/get_parameters', request).values
    p = dict(zip(names, values))
    fk = GetPositionFK.Request()
    fk.header.frame_id = p['base_frame'].string_value
    fk.fk_link_names = [p['tip_link'].string_value]
    fk.robot_state.joint_state.name = [f'link_{i}_joint' for i in range(1, 7)]
    fk.robot_state.joint_state.position = [.25, .2, -.3, .2, .4, .1]
    result = call(node, GetPositionFK, '/compute_fk', fk)
    if result.error_code.val != 1 or not result.pose_stamped:
        raise RuntimeError('无法求出示例关节目标的 FK')
    pregrasp = pose_matrix(result.pose_stamped[0].pose)
    object_tcp = xyz_rpy(p['object_to_tcp_xyz'].double_array_value,
                         p['object_to_tcp_rpy'].double_array_value)
    tip_tcp = xyz_rpy(p['tip_to_tcp_xyz'].double_array_value,
                      p['tip_to_tcp_rpy'].double_array_value)
    forward = np.eye(4)
    axis = np.array(p['approach_axis'].double_array_value)
    forward[:3, 3] = p['pregrasp_distance'].double_value * axis / np.linalg.norm(axis)
    # 反解几何公式，使最终预抓取 link_6 目标恰好等于上面的 FK。
    message = matrix_pose(pregrasp @ tip_tcp @ forward @ np.linalg.inv(object_tcp))
    message.header.frame_id = fk.header.frame_id
    return message, pregrasp


def main():
    rclpy.init()
    node = rclpy.create_node('mtc_demo_target')
    status = 0
    try:
        message, pregrasp = demo_target(node)
        publisher = node.create_publisher(PoseStamped, '/engineer_mtc/manual_object_pose', 1)
        deadline = time.monotonic() + 5
        while publisher.get_subscription_count() == 0:
            if time.monotonic() > deadline: raise RuntimeError('MTC 手动目标订阅未就绪')
            rclpy.spin_once(node, timeout_sec=.1)
        message.header.stamp = node.get_clock().now().to_msg()
        publisher.publish(message)
        deadline = time.monotonic() + .15
        while time.monotonic() < deadline: rclpy.spin_once(node, timeout_sec=.02)
        print('虚拟矿由 FK 反推，用于演示，不是真实视觉或标定数据。')
        print('预抓取 link_6 目标位置（m）：', pregrasp[:3, 3].tolist())
        response = call(node, Trigger, '/engineer_mtc/plan_manual', Trigger.Request())
        print(response.message)
        status = 0 if response.success else 1
    except Exception as error:
        print(f'MTC 示例失败：{error}')
        status = 1
    finally:
        node.destroy_node()
        rclpy.shutdown()
    return status


if __name__ == '__main__':
    raise SystemExit(main())
