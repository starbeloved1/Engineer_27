#!/usr/bin/env python3
"""从单块矿 STL 新建 URDF：两端圆柱、中段长方体，保留窄腰。

仅需 NumPy。原 STL/PLY 不修改；输出 energy.urdf 和几何校验报告。
模型尺寸来自现有网格，不是实物测量或新的赛季尺寸定义。
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import xml.etree.ElementTree as ET

import numpy as np


def read_stl(path):
    data = path.read_bytes()
    count = struct.unpack_from('<I', data, 80)[0]
    if len(data) != 84 + 50 * count:
        raise ValueError('Expected binary STL')
    dtype = np.dtype([('normal', '<f4', (3,)), ('vertices', '<f4', (3, 3)),
                      ('attribute', '<u2')])
    triangles = np.frombuffer(data, dtype=dtype, offset=84)['vertices'].astype(float)
    if not len(triangles) or not np.isfinite(triangles).all():
        raise ValueError('Invalid mesh')
    return triangles


def clip_polygon(polygon, z, above):
    result = []
    for i, start in enumerate(polygon):
        end = polygon[(i + 1) % len(polygon)]
        inside_start = start[2] >= z if above else start[2] <= z
        inside_end = end[2] >= z if above else end[2] <= z
        if inside_start:
            result.append(start)
        if inside_start != inside_end:
            result.append(start + (z - start[2]) / (end[2] - start[2]) * (end - start))
    return result


def slab_points(triangles, lower, upper):
    """逐三角形按 Z 裁剪，包含切面交点，避免只检查原始顶点。"""
    polygons = []
    for triangle in triangles:
        polygon = clip_polygon(list(triangle), lower, True)
        polygon = clip_polygon(polygon, upper, False) if polygon else []
        if polygon:
            polygons.extend(polygon)
    if not polygons:
        raise ValueError('Empty axial segment')
    return np.array(polygons)


def numbers(values):
    return ' '.join(f'{float(v):.10f}' for v in np.atleast_1d(values))


def generate(directory, margin_mm=0.2):
    if not np.isfinite(margin_mm) or margin_mm < 0:
        raise ValueError('margin-mm must be finite and nonnegative')
    source = directory / 'energy.stl'
    triangles = read_stl(source)
    bounds = np.array([triangles.min(axis=(0, 1)), triangles.max(axis=(0, 1))])
    # 在直腰内选取分界；两端包括肩部。Z=40..110 mm 为净抓取段。
    cuts = [bounds[0, 2], 40., 110., bounds[1, 2]]
    if not all(b > a for a, b in zip(cuts, cuts[1:])):
        raise ValueError('Unexpected model dimensions; review segment cuts')
    segments = []
    for i, name in enumerate(['wide_end', 'waist', 'small_end']):
        points = slab_points(triangles, cuts[i], cuts[i + 1])
        lo, hi = points.min(axis=0), points.max(axis=0)
        center = (lo + hi) / 2
        center[2] = (cuts[i] + cuts[i + 1]) / 2
        segment = {'name': name, 'kind': 'box' if i == 1 else 'cylinder',
                   'center_mesh_mm': center.tolist(),
                   'z_range_mesh_mm': cuts[i:i + 2]}
        if i == 1:
            size = hi - lo + 2 * margin_mm
            segment['size_mm'] = size.tolist()
            slack = float(np.min(size / 2 - abs(points - center)))
        else:
            radius = float(np.linalg.norm(points[:, :2] - center[:2], axis=1).max()
                           + margin_mm)
            length = cuts[i + 1] - cuts[i] + 2 * margin_mm
            segment.update(radius_mm=radius, length_mm=length)
            slack = min(float((radius - np.linalg.norm(points[:, :2] - center[:2],
                                                       axis=1)).min()),
                        float((length / 2 - abs(points[:, 2] - center[2])).min()))
        # 每块为凸几何：包住裁剪多边形所有顶点即包住该段整个三角面。
        if slack < -1e-8:
            raise ValueError(f'Collision segment misses mesh: {name}')
        segment.update(clipped_vertices=len(points), min_cover_slack_mm=slack)
        segments.append(segment)

    origin = np.array(segments[1]['center_mesh_mm'])
    robot = ET.Element('robot', name='energy')
    robot.append(ET.Comment('Standalone rigid object; not a calibrated gripper TCP. Units: m.'))
    link = ET.SubElement(robot, 'link', name='energy_link')
    visual = ET.SubElement(link, 'visual', name='energy_visual')
    ET.SubElement(visual, 'origin', xyz=numbers(-origin * .001), rpy='0 0 0')
    geometry = ET.SubElement(visual, 'geometry')
    ET.SubElement(geometry, 'mesh', filename='energy.stl', scale='0.001 0.001 0.001')
    material = ET.SubElement(visual, 'material', name='energy_silver')
    ET.SubElement(material, 'color', rgba='0.70 0.76 0.82 1')
    for segment in segments:
        collision = ET.SubElement(link, 'collision', name=segment['name'])
        center = (np.array(segment['center_mesh_mm']) - origin) * .001
        ET.SubElement(collision, 'origin', xyz=numbers(center), rpy='0 0 0')
        geometry = ET.SubElement(collision, 'geometry')
        if segment['kind'] == 'box':
            ET.SubElement(geometry, 'box', size=numbers(np.array(segment['size_mm']) * .001))
        else:
            ET.SubElement(geometry, 'cylinder', radius=numbers(segment['radius_mm'] * .001),
                          length=numbers(segment['length_mm'] * .001))
    ET.indent(robot, space='  ')
    ET.ElementTree(robot).write(directory / 'energy.urdf', encoding='utf-8', xml_declaration=True)
    report = {
        'source': 'energy.stl', 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
        'source_triangles': len(triangles), 'source_bounds_mm': bounds.tolist(),
        'energy_link_origin_in_mesh_mm': origin.tolist(),
        'energy_link_axes': 'Same XYZ directions as energy.stl; +Z toward small end',
        'margin_mm': margin_mm, 'segments': segments,
        'validation': 'Every STL triangle clipped at segment planes; all clipped polygon '
                      'vertices enclosed by its convex primitive, therefore all source '
                      'triangle surfaces are enclosed (floating-point tolerance 1e-8 mm).',
        'limitations': ['External rigid grasp model, not physical dynamics.',
                        'End bores, chamfers and shoulder free spaces are filled.',
                        'PLY-to-energy_link rigid transform must be established before '
                        'placing this URDF at an ICP pose; bounding boxes do not calibrate it.',
                        'Numerical margin is not a sensor or mechanical uncertainty budget.'],
    }
    (directory / 'collision_report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--energy-dir', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'assets/energy')
    parser.add_argument('--margin-mm', type=float, default=0.2)
    args = parser.parse_args()
    generate(args.energy_dir.resolve(), args.margin_mm)
