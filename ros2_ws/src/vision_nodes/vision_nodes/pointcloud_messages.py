"""PointCloud2 与 NumPy 之间转换，保留 RGB，支持行填充及大小端。"""
import numpy as np
from sensor_msgs.msg import PointCloud2, PointField
from sensor_msgs_py.point_cloud2 import create_cloud, dtype_from_fields


def read_cloud(message):
    if message.width == 0 or message.height == 0:
        return np.empty((0, 3)), None
    dtype = dtype_from_fields(message.fields, message.point_step).newbyteorder(
        '>' if message.is_bigendian else '<')
    if not {'x', 'y', 'z'}.issubset(dtype.names):
        raise ValueError("PointCloud2 缺少 x/y/z 字段")
    points = np.ndarray((message.height, message.width), dtype=dtype,
                        buffer=message.data,
                        strides=(message.row_step, message.point_step)).reshape(-1)
    xyz = np.column_stack([points[name] for name in ('x', 'y', 'z')]).astype(np.float64)
    rgb = None
    color_field = next((name for name in ('rgb', 'rgba') if name in dtype.names), None)
    if color_field is not None:
        color = points[color_field]
        if color.dtype.kind == 'f':
            packed = color.view(color.dtype.str.replace('f', 'u')).astype(np.uint32)
        else:
            packed = color.astype(np.uint32)
        rgb = np.column_stack([(packed >> shift) & 255 for shift in (16, 8, 0)]).astype(np.uint8)
    return xyz, rgb


def make_cloud(header, xyz, rgb=None):
    xyz = np.asarray(xyz, dtype=np.float32).reshape(-1, 3)
    fields = [PointField(name=name, offset=i * 4, datatype=PointField.FLOAT32, count=1)
              for i, name in enumerate(('x', 'y', 'z'))]
    if rgb is None:
        return create_cloud(header, fields, xyz)
    rgb = np.asarray(rgb, dtype=np.uint32).reshape(-1, 3)
    if len(rgb) != len(xyz):
        raise ValueError("点坐标与 RGB 数量不一致")
    fields.append(PointField(name='rgb', offset=12, datatype=PointField.FLOAT32, count=1))
    data = np.empty(len(xyz), dtype=dtype_from_fields(fields))
    for i, name in enumerate(('x', 'y', 'z')):
        data[name] = xyz[:, i]
    packed = (rgb[:, 0] << 16) | (rgb[:, 1] << 8) | rgb[:, 2]
    data['rgb'] = packed.view(np.float32)
    return create_cloud(header, fields, data)
