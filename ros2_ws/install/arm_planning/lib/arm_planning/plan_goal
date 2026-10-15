#!/usr/bin/python3
"""Submit a joint or link_6 pose goal; optionally execute on the offline demo."""
import argparse
import json
from pathlib import Path
import sys

import rclpy
from rclpy.action import ActionClient
from rclpy.utilities import remove_ros_args
from arm_planning.action import PlanMotion


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    target = parser.add_mutually_exclusive_group(required=True)
    target.add_argument('--joints', type=float, nargs=6, metavar=('J1', 'J2', 'J3', 'J4', 'J5', 'J6'))
    target.add_argument('--pose', type=float, nargs=7, metavar=('X', 'Y', 'Z', 'QX', 'QY', 'QZ', 'QW'))
    parser.add_argument('--frame', default='base_link', choices=['base_link', 'world'])
    parser.add_argument('--execute', action='store_true')
    parser.add_argument('--planning-time', type=float, default=5.)
    parser.add_argument('--velocity-scale', type=float, default=.2)
    parser.add_argument('--acceleration-scale', type=float, default=.2)
    parser.add_argument('--timeout', type=float, default=120.)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args(remove_ros_args()[1:])
    rclpy.init()
    node = rclpy.create_node('arm_plan_goal')
    client = ActionClient(node, PlanMotion, '/arm_planning/plan_motion')
    handle = None
    try:
        if not client.wait_for_server(timeout_sec=20.):
            raise RuntimeError('Planning server unavailable; start engineer27_moveit_config demo.launch.py')
        goal = PlanMotion.Goal()
        if args.joints is not None:
            goal.target_type = goal.JOINT
            goal.joint_positions = args.joints
        else:
            goal.target_type = goal.POSE
            goal.target_pose.header.frame_id = args.frame
            p, q = goal.target_pose.pose.position, goal.target_pose.pose.orientation
            p.x, p.y, p.z, q.x, q.y, q.z, q.w = args.pose
        goal.execute = args.execute
        goal.planning_time = args.planning_time
        goal.velocity_scaling = args.velocity_scale
        goal.acceleration_scaling = args.acceleration_scale
        def feedback(message):
            print(f'{message.feedback.stage}: {message.feedback.detail}', flush=True)
        sent = client.send_goal_async(goal, feedback_callback=feedback)
        rclpy.spin_until_future_complete(node, sent, timeout_sec=20.)
        if not sent.done():
            raise RuntimeError('Goal acceptance timed out')
        handle = sent.result()
        if not handle.accepted:
            raise RuntimeError('Goal rejected: server busy, invalid target/frame, or invalid request limits')
        future = handle.get_result_async()
        rclpy.spin_until_future_complete(node, future, timeout_sec=args.timeout)
        if not future.done():
            canceled = handle.cancel_goal_async()
            rclpy.spin_until_future_complete(node, canceled, timeout_sec=3.)
            raise RuntimeError('Request timed out; cancellation requested')
        result = future.result().result
        trajectory = result.trajectory.joint_trajectory
        data = {'success': result.success, 'error_code': result.error_code, 'message': result.message,
                'selected_joint_positions': list(result.selected_joint_positions),
                'planning_time': result.planning_time, 'joint_path_length': result.joint_path_length,
                'validation_samples': result.validation_samples, 'ik_candidates': result.ik_candidates,
                'joint_names': list(trajectory.joint_names), 'points': [
                    {'positions': list(p.positions), 'velocities': list(p.velocities),
                     'accelerations': list(p.accelerations),
                     'time_from_start': p.time_from_start.sec + p.time_from_start.nanosec * 1e-9}
                    for p in trajectory.points]}
        print(json.dumps({k: v for k, v in data.items() if k != 'points'}, indent=2))
        print(f'Trajectory points: {len(trajectory.points)}')
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(json.dumps(data, indent=2) + '\n')
        return 0 if result.success else 1
    except KeyboardInterrupt:
        if handle is not None and rclpy.ok():
            future = handle.cancel_goal_async()
            rclpy.spin_until_future_complete(node, future, timeout_sec=3.)
        return 130
    except RuntimeError as error:
        print(str(error), file=sys.stderr)
        return 2
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    sys.exit(main())
