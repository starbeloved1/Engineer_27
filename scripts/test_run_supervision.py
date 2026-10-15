#!/usr/bin/python3
"""验证 run.sh 的真实 launch 进程管理；在独立 ROS_DOMAIN_ID 中运行。"""
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parent.parent


def children(pid):
    try:
        return [int(value) for value in Path(f'/proc/{pid}/task/{pid}/children').read_text().split()]
    except FileNotFoundError:
        return []


def descendants(pid):
    result = set()
    for child in children(pid):
        result.add(child)
        result.update(descendants(child))
    return result


def running(pid):
    try:
        return Path(f'/proc/{pid}/stat').read_text().rsplit(')', 1)[1].split()[0] != 'Z'
    except FileNotFoundError:
        return False


class RunSupervision(unittest.TestCase):
    def run_system(self, fail_launch):
        environment = dict(os.environ, ROS_DOMAIN_ID=os.environ.get('ROS_DOMAIN_ID', '195'))
        with tempfile.TemporaryDirectory(prefix='engineer_run_') as directory:
            path = Path(directory) / 'runtime.log'
            with path.open('w') as log:
                process = subprocess.Popen([str(ROOT / 'run.sh'), 'use_rviz:=false'], cwd=ROOT,
                                           env=environment, stdout=log, stderr=subprocess.STDOUT,
                                           start_new_session=True)
                tracked = set()
                try:
                    deadline = time.monotonic() + 40
                    while time.monotonic() < deadline:
                        self.assertIsNone(process.poll(), path.read_text())
                        if 'Ready: /arm_planning/plan_motion' in path.read_text():
                            break
                        time.sleep(.1)
                    else:
                        self.fail('Planning server did not become ready: ' + path.read_text())
                    tracked = descendants(process.pid)
                    launches = children(process.pid)
                    self.assertEqual(len(launches), 4, path.read_text())
                    if fail_launch:
                        target = next(pid for pid in launches if b'arm_goal.launch.py' in
                                      Path(f'/proc/{pid}/cmdline').read_bytes())
                        os.kill(target, signal.SIGTERM)
                    else:
                        process.send_signal(signal.SIGINT)
                    code = process.wait(timeout=20)
                    if fail_launch:
                        self.assertNotEqual(code, 0, path.read_text())
                        self.assertIn('ROS launch 进程已退出', path.read_text())
                    else:
                        self.assertEqual(code, 0, path.read_text())
                    remaining = [pid for pid in tracked if running(pid)]
                    self.assertEqual(remaining, [], path.read_text())
                    self.assertNotIn('Segmentation fault', path.read_text())
                finally:
                    if process.poll() is None:
                        process.send_signal(signal.SIGINT)
                        try:
                            process.wait(timeout=20)
                        except subprocess.TimeoutExpired:
                            # 仅清理本测试启动的进程，不按节点名或包名批量结束外部进程。
                            for pid in descendants(process.pid):
                                try:
                                    os.kill(pid, signal.SIGKILL)
                                except ProcessLookupError:
                                    pass
                            process.kill()
                            process.wait()

    def test_ctrl_c_stops_all_owned_launches_and_nodes(self):
        self.run_system(fail_launch=False)

    def test_launch_exit_stops_the_rest(self):
        self.run_system(fail_launch=True)

    def test_invalid_argument_does_not_start_system(self):
        result = subprocess.run([str(ROOT / 'run.sh'), 'use_rviz:=invalid'], cwd=ROOT,
                                capture_output=True, text=True, timeout=5)
        self.assertEqual(result.returncode, 2)
        self.assertIn('未知或无效参数', result.stderr)


if __name__ == '__main__':
    unittest.main(verbosity=2)
