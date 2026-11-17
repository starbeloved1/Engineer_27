"""根据实际 MoveIt 场景显示矿；附着时使用末端 TF，不另做运动动画。"""
from pathlib import Path
import xml.etree.ElementTree as ET

from ament_index_python.packages import get_package_share_directory
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, DurabilityPolicy
from moveit_msgs.msg import PlanningSceneComponents
from moveit_msgs.srv import GetPlanningScene
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Pose


def compose(a, b):
    # xyzw 四元数乘法及旋转；避免为显示节点引入额外几何库。
    def multiply(q, p):
        x, y, z, w = q
        u, v, s, t = p
        return (w*u+x*t+y*s-z*v, w*v-x*s+y*t+z*u,
                w*s+x*v-y*u+z*t, w*t-x*u-y*v-z*s)
    q = (a.orientation.x, a.orientation.y, a.orientation.z, a.orientation.w)
    p = (b.orientation.x, b.orientation.y, b.orientation.z, b.orientation.w)
    v = (b.position.x, b.position.y, b.position.z, 0.)
    xyz = multiply(multiply(q, v), (-q[0], -q[1], -q[2], q[3]))
    out = Pose()
    out.position.x, out.position.y, out.position.z = (a.position.x+xyz[0],
                                                     a.position.y+xyz[1], a.position.z+xyz[2])
    out.orientation.x, out.orientation.y, out.orientation.z, out.orientation.w = multiply(q, p)
    return out


class EnergyDisplay(Node):
    def __init__(self):
        super().__init__('energy_display')
        self.object_id = self.declare_parameter('object_id', 'energy').value
        share = Path(get_package_share_directory('moveit_config'))
        root = ET.parse(share / 'energy/energy.urdf').getroot()
        visual = root.find('link/visual')
        self.local = Pose()
        xyz = [float(v) for v in visual.find('origin').get('xyz').split()]
        self.local.position.x, self.local.position.y, self.local.position.z = xyz
        # 当前 energy.urdf 的 visual 无旋转，遇到修改时明确拒绝而不是静默画错。
        if any(float(v) for v in visual.find('origin').get('rpy', '0 0 0').split()):
            raise ValueError('energy visual rotation must be zero; update display composition first')
        self.local.orientation.w = 1.
        mesh = visual.find('geometry/mesh')
        self.mesh = mesh.get('filename')
        self.scale = [float(v) for v in mesh.get('scale', '1 1 1').split()]
        self.publisher = self.create_publisher(Marker, '~/marker',
            QoSProfile(depth=1, durability=DurabilityPolicy.TRANSIENT_LOCAL))
        self.client = self.create_client(GetPlanningScene, '/get_planning_scene')
        self.pending = None
        self.timer = self.create_timer(.2, self.query)
        self.marker(False)

    def query(self):
        if self.pending is not None:
            return
        if not self.client.service_is_ready():
            return
        request = GetPlanningScene.Request()
        request.components.components = (PlanningSceneComponents.WORLD_OBJECT_GEOMETRY |
                                          PlanningSceneComponents.ROBOT_STATE_ATTACHED_OBJECTS)
        self.pending = self.client.call_async(request)
        self.pending.add_done_callback(self.update)

    def update(self, future):
        self.pending = None
        try:
            scene = future.result().scene
            for attached in scene.robot_state.attached_collision_objects:
                if attached.object.id == self.object_id:
                    self.marker(True, attached.object.pose, attached.link_name, True)
                    return
            for obj in scene.world.collision_objects:
                if obj.id == self.object_id:
                    self.marker(True, obj.pose, obj.header.frame_id, False)
                    return
            self.marker(False)
        except Exception as error:
            self.get_logger().warning(f'矿场景读取失败: {error}')

    def marker(self, visible, pose=None, frame='world', attached=False):
        m = Marker()
        m.header.frame_id = frame
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'energy'
        m.id = 0
        m.action = Marker.ADD if visible else Marker.DELETE
        if visible:
            m.type = Marker.MESH_RESOURCE
            m.mesh_resource = self.mesh
            m.mesh_use_embedded_materials = False
            m.pose = compose(pose, self.local)
            m.scale.x, m.scale.y, m.scale.z = self.scale
            m.color.r, m.color.g, m.color.b, m.color.a = .70, .76, .82, 1.
            m.frame_locked = attached
        self.publisher.publish(m)


def main():
    rclpy.init()
    node = EnergyDisplay()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
