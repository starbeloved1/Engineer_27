#!/usr/bin/env python3
"""Build independent conservative collision covers from original CAD triangles.

Each original triangle belongs to one convex piece enclosing all three vertices.
Concave parts are split by triangle groups, so this is not a whole-link convex
hull. Covers include a 0.2 mm numerical margin. Sampled excess distances are
diagnostics; the CAD export and real robot still need engineering review.
"""
from __future__ import annotations

import argparse
import copy
import json
import os
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

import numpy as np
from scipy.spatial import ConvexHull, HalfspaceIntersection
from scipy.sparse import coo_matrix
from scipy.sparse.csgraph import connected_components
import trimesh

import merge_link_meshes as merger
import simplify_visual_meshes as visual

MARGIN = 0.0002
EXCESS_TARGET = 0.002
MAX_DEPTH = 5
SAMPLES = 128


def exterior_envelope_part(name):
    """Explicit parts where mounting bores/internal electronics are not free space.

    Structural brackets and gripper parts always take precedence over this list.
    This policy is reported so it can be reviewed against the actual task.
    """
    if any(token in name for token in ('夹爪', '触手', '柔性', 'U形', '安装', '支架', '连接', '保护', '防护', '连杆', '固定')):
        return False
    return any(token in name for token in (
        '轴承', '同步带轮', 'GB_', 'M2_', 'M3_', 'M4_', 'M5_', 'M6_', 'DM_', 'dm_',
        'n_4340', 'M3508', 'RM3508', 'RM裁判系统', 'PCB_', '电池', '麦轮', '全向轮', '滚子',
        '避震器', 'ZHUTI', 'HOUGAI', 'n_1_0_stp', 'n_2_0_stp',
        'n_2020_', 'n_4020_', 'n_4030_', 'n_2040_', 'n_3030_', 'n_4040_',
    ))


def convex_cover(triangles: np.ndarray) -> tuple[trimesh.Trimesh, dict]:
    points = np.unique(triangles.reshape(-1, 3), axis=0).astype(float)
    center = points.mean(axis=0)
    _, frame = np.linalg.eigh((points - center).T @ (points - center))
    if np.linalg.det(frame) < 0:
        frame[:, 0] *= -1
    local = (points - center) @ frame
    lower, upper = local.min(axis=0), local.max(axis=0)
    original_hull = None
    try:
        original_hull = ConvexHull(points)
    except Exception:
        pass
    if original_hull is not None and original_hull.volume > 1e-15:
        # An oriented box is both cheaper and more accurate for rectangular parts.
        if np.prod(upper - lower) / original_hull.volume <= 1.08:
            transform = np.eye(4)
            transform[:3, :3] = frame
            transform[:3, 3] = center + frame @ ((lower + upper) / 2)
            mesh = trimesh.creation.box(extents=upper - lower + 2 * MARGIN, transform=transform)
            return verify_cover(mesh, points, len(triangles), 'oriented_box')
        cylinders = []
        for axis in range(3):
            radial = [i for i in range(3) if i != axis]
            radial_center = (lower[radial] + upper[radial]) / 2
            radius = np.linalg.norm(local[:, radial] - radial_center, axis=1).max()
            height = upper[axis] - lower[axis]
            cylinders.append((np.pi * radius**2 * height / original_hull.volume, axis, radius, height))
        ratio, axis, radius, height = min(cylinders)
        if ratio <= 1.10 and height > 0:
            radial = [i for i in range(3) if i != axis]
            rotation = frame[:, radial + [axis]].copy()
            if np.linalg.det(rotation) < 0:
                rotation[:, 0] *= -1
            transform = np.eye(4)
            transform[:3, :3] = rotation
            transform[:3, 3] = center + frame @ ((lower + upper) / 2)
            mesh = trimesh.creation.cylinder(radius=(radius + MARGIN) / np.cos(np.pi / 24),
                                            height=height + 2 * MARGIN, sections=24, transform=transform)
            return verify_cover(mesh, points, len(triangles), 'circumscribed_cylinder_24')
    directions = np.asarray([(x, y, z) for x in (-1, 0, 1) for y in (-1, 0, 1)
                             for z in (-1, 0, 1) if x or y or z], dtype=float)
    directions /= np.linalg.norm(directions, axis=1)[:, None]
    normals = np.vstack([directions @ frame.T, np.eye(3), -np.eye(3)])
    # Preserve dominant oblique planar faces, rather than rounding plates into boxes.
    try:
        if original_hull is None:
            original_hull = ConvexHull(points)
        faces = points[original_hull.simplices]
        area = np.linalg.norm(np.cross(faces[:, 1] - faces[:, 0], faces[:, 2] - faces[:, 0]), axis=1)
        selected = []
        for i in np.argsort(-area, kind='stable'):
            n = original_hull.equations[i, :3]
            if all(abs(np.dot(n, previous)) < .9999 for previous in selected):
                selected.append(n)
            if len(selected) >= 12:
                break
        if selected:
            normals = np.vstack([normals, selected, -np.asarray(selected)])
    except Exception:
        pass  # A flat CAD surface can still be covered by the padded support planes.
    supports = np.max(points @ normals.T, axis=0) + MARGIN
    halfspaces = np.column_stack([normals, -supports])
    vertices = HalfspaceIntersection(halfspaces, center).intersections
    hull = ConvexHull(vertices)
    faces = hull.simplices.copy()
    face_points = vertices[faces]
    face_normals = np.cross(face_points[:, 1] - face_points[:, 0], face_points[:, 2] - face_points[:, 0])
    reversed_faces = np.einsum('ij,ij->i', face_normals, hull.equations[:, :3]) < 0
    faces[reversed_faces] = faces[reversed_faces][:, [0, 2, 1]]
    # Pad to contain float32 serialization error too; coordinates here are part-local.
    mesh = trimesh.Trimesh(vertices.astype(np.float32), faces, process=False)
    return verify_cover(mesh, points, len(triangles), 'support_plane_polytope')


def verify_cover(mesh, points, count, kind):
    mesh.vertices = np.asarray(mesh.vertices, dtype=np.float32)
    output_hull = ConvexHull(mesh.vertices)
    violation = float(np.max(points @ output_hull.equations[:, :3].T + output_hull.equations[:, 3]))
    if violation > 1e-7:
        raise ValueError(f'Collision cover misses source vertices: {violation} m')
    return mesh, {'source_triangles': count, 'source_vertices': len(points), 'kind': kind,
                  'max_source_vertex_plane_violation_m': violation, 'triangles': len(mesh.faces)}


def excess_distance(cover: trimesh.Trimesh, source: trimesh.Trimesh) -> dict:
    points, _ = trimesh.sample.sample_surface(cover, SAMPLES, seed=42)
    points = np.vstack([points, cover.triangles_center])
    distances = []
    for i in range(0, len(points), 128):
        _, d, _ = trimesh.proximity.closest_point(source, points[i:i + 128])
        distances.append(d)
    d = np.concatenate(distances)
    return {'max_sample_distance_m': float(d.max()), 'p95_sample_distance_m': float(np.percentile(d, 95)),
            'samples': len(d)}


def build_part(path: Path) -> tuple[list[np.ndarray], dict]:
    source = visual.indexed_mesh(path)
    triangles = source.triangles
    pieces, diagnostics = [], []

    def recurse(group, depth):
        cover, info = convex_cover(group)
        excess = excess_distance(cover, source)
        if excess['max_sample_distance_m'] > EXCESS_TARGET and depth < MAX_DEPTH and len(group) > 16:
            centroids = group.mean(axis=1)
            _, frame = np.linalg.eigh(np.cov(centroids.T))
            choices = []
            # Favor splits that remove empty volume (openings and U-shaped parts).
            for axis in frame.T:
                projection = centroids @ axis
                order = np.argsort(projection, kind='stable')
                middle = len(order) // 2
                left, right = group[order[:middle]], group[order[middle:]]
                a, _ = convex_cover(left)
                b, _ = convex_cover(right)
                choices.append((abs(a.volume) + abs(b.volume), left, right))
            _, left, right = min(choices, key=lambda item: item[0])
            recurse(left, depth + 1)
            recurse(right, depth + 1)
        else:
            pieces.append(cover.triangles.astype(np.float32))
            diagnostics.append({**info, **excess, 'depth': depth,
                                'sample_target_met': excess['max_sample_distance_m'] <= EXCESS_TARGET})

    # Disconnected CAD solids (motor housings, screws, boards) get independent
    # covers. This avoids filling the spaces between assembly components.
    edges = source.edges
    graph = coo_matrix((np.ones(len(edges)), (edges[:, 0], edges[:, 1])), shape=(len(source.vertices),) * 2)
    _, labels = connected_components(graph, directed=False)
    face_labels = labels[source.faces[:, 0]]
    for label in np.unique(face_labels):
        group = triangles[face_labels == label]
        if exterior_envelope_part(path.name):
            cover, info = convex_cover(group)
            excess = excess_distance(cover, source)
            pieces.append(cover.triangles.astype(np.float32))
            diagnostics.append({**info, **excess, 'depth': 0,
                                'sample_target_met': excess['max_sample_distance_m'] <= EXCESS_TARGET, 'exterior_envelope_policy': True,
                                'note': 'Internal mounting bores/voids intentionally filled; sampled distance to all CAD surfaces is diagnostic only.'})
            continue
        old_count = len(pieces)
        recurse(group, 0)
        new_info = diagnostics[old_count:]
        # Small difficult concave solids keep their exact original surface.
        # This protects gripper slots/openings and avoids a larger approximate mesh.
        if len(group) <= 25000 and (
            any(not p['sample_target_met'] for p in new_info)
            or sum(p['triangles'] for p in new_info) >= len(group)
        ):
            del pieces[old_count:]
            del diagnostics[old_count:]
            pieces.append(group.astype(np.float32))
            diagnostics.append({'source_triangles': len(group), 'triangles': len(group),
                                'kind': 'original_concave_surface', 'depth': 0,
                                'max_sample_distance_m': 0., 'p95_sample_distance_m': 0.,
                                'max_source_vertex_plane_violation_m': 0.,
                                'samples': 0, 'sample_target_met': True})
    report = {'source_file': path.name, 'source_triangles': merger.triangle_count(path),
              'nondegenerate_source_triangles': len(triangles), 'pieces': diagnostics,
              'piece_count': len(pieces), 'triangles': sum(len(p) for p in pieces),
              'sample_target_met': all(p['sample_target_met'] for p in diagnostics),
              'exterior_envelope_policy': exterior_envelope_part(path.name),
              'max_sample_distance_m': max(p['max_sample_distance_m'] for p in diagnostics),
              'all_source_triangles_covered': sum(p['source_triangles'] for p in diagnostics) == len(triangles)}
    if not report['all_source_triangles_covered']:
        raise ValueError('Lost original triangles during collision decomposition')
    return pieces, report


def without_collision(root):
    result = copy.deepcopy(root)
    for link in result.findall('link'):
        for collision in link.findall('collision'):
            link.remove(collision)
    return merger.semantic(result)


def publish(assets, staging):
    targets = ['meshes/collision', 'arm.urdf', 'collision_report.json']
    saved, installed = [], []
    try:
        for relative in targets:
            destination = assets / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            if destination.exists():
                backup = staging / 'previous' / relative
                backup.parent.mkdir(parents=True, exist_ok=True)
                os.replace(destination, backup)
                saved.append(relative)
            os.replace(staging / relative, destination)
            installed.append(relative)
    except BaseException:
        for relative in reversed(installed):
            target = staging / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            os.replace(assets / relative, target)
        for relative in reversed(saved):
            os.replace(staging / 'previous' / relative, assets / relative)
        raise


def generate(assets: Path) -> dict:
    assets = assets.resolve()
    runtime_dir = merger.runtime_directory(assets)
    merger.ensure_archive(assets)
    source_file = assets / 'source' / 'arm.urdf'
    source = ET.parse(source_file).getroot()
    runtime = ET.parse(runtime_dir / 'arm.urdf').getroot()
    runtime_snapshot = merger.snapshot(runtime_dir / 'arm.urdf')
    original_runtime = copy.deepcopy(runtime)
    runtime_links = {l.get('name'): l for l in runtime.findall('link')}
    cache = {}
    report = {'operation': 'independent_collision_geometry', 'source': 'source/arm.urdf',
              'algorithm': 'Per-connected-solid box/cylinder/support-plane covers; adaptive triangle splits; original small concave surfaces retained',
              'numerical_margin_m': MARGIN, 'sample_excess_target_m': EXCESS_TARGET,
              'max_split_depth': MAX_DEPTH, 'links': {}, 'parts': {},
              'limitations': ['Sampled surface distances are not exhaustive error bounds.',
                              'Conservative covers can fill small holes and cause false positive collisions.',
                              'Listed motors/bearings/electronics/extrusions use exterior envelopes; their internal cavities are intentionally occupied.',
                              'No mechanical joint limits, moving fingers or SRDF exclusions are added.']}
    with tempfile.TemporaryDirectory(prefix='.collision-', dir=assets) as directory:
        staging = Path(directory)
        meshes = staging / 'meshes' / 'collision'
        meshes.mkdir(parents=True)
        for link in source.findall('link'):
            name = link.get('name')
            if name not in runtime_links:
                raise ValueError(f'Missing runtime link: {name}')
            transformed, instance_count, piece_count = [], 0, 0
            specs = merger.visual_specs(link, source_file)
            for path, count, R, xyz, scale in specs:
                if path not in cache:
                    parts, info = build_part(path)
                    cache[path] = parts, info
                    report['parts'][path.name] = info
                    print(f'{name}: {path.name}: {count:,} -> {info["triangles"]:,} faces / {len(parts)} pieces', flush=True)
                parts, _ = cache[path]
                transformed.extend(((part.astype(float) * scale) @ R.T + xyz).astype(np.float32) for part in parts)
                instance_count += 1
                piece_count += len(parts)
            output = meshes / f'{name}.stl'
            visual.write_stl(output, transformed, name)
            # A padded closed cover must enclose the source link's bounding range.
            bounds = np.array([np.minimum.reduce([p.min(axis=(0, 1)) for p in transformed]),
                               np.maximum.reduce([p.max(axis=(0, 1)) for p in transformed])])
            original_bounds = [np.full(3, np.inf), np.full(3, -np.inf)]
            for spec in specs:
                for records in merger.chunks(spec, 100000):
                    points = merger.expected_vertices(records, spec)
                    original_bounds[0] = np.minimum(original_bounds[0], points.min(axis=(0, 1)))
                    original_bounds[1] = np.maximum(original_bounds[1], points.max(axis=(0, 1)))
            if np.any(bounds[0] > original_bounds[0] + 1e-7) or np.any(bounds[1] < original_bounds[1] - 1e-7):
                raise ValueError(f'Collision bounds fail to enclose original: {name}')
            report['links'][name] = {'original_instances': instance_count, 'convex_pieces': piece_count,
                                      'triangles': merger.triangle_count(output), 'bytes': output.stat().st_size,
                                      'source_bounds_m': np.asarray(original_bounds).tolist(), 'collision_bounds_m': bounds.tolist()}
            target = runtime_links[name]
            for collision in target.findall('collision'):
                target.remove(collision)
            collision = ET.SubElement(target, 'collision', {'name': f'{name}_collision'})
            ET.SubElement(collision, 'origin', {'xyz': '0 0 0', 'rpy': '0 0 0'})
            geometry = ET.SubElement(collision, 'geometry')
            ET.SubElement(geometry, 'mesh', {'filename': f'meshes/collision/{name}.stl'})
            print(f'{name}: {instance_count} instances / {piece_count} pieces / {report["links"][name]["triangles"]:,} collision faces', flush=True)
        if without_collision(runtime) != without_collision(original_runtime):
            raise ValueError('Configuration other than collision changed')
        ET.indent(runtime, space='  ')
        ET.ElementTree(runtime).write(staging / 'arm.urdf', encoding='utf-8', xml_declaration=True)
        checked = subprocess.run(['/opt/ros/humble/bin/check_urdf', str(staging / 'arm.urdf')], capture_output=True, text=True, timeout=30)
        if checked.returncode:
            raise ValueError(checked.stdout + checked.stderr)
        report['total_triangles'] = sum(v['triangles'] for v in report['links'].values())
        report['total_bytes'] = sum(v['bytes'] for v in report['links'].values())
        report['parts_above_sample_target'] = [name for name, v in report['parts'].items() if not v['sample_target_met']]
        (staging / 'collision_report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
        merger.ensure_archive(assets)
        if merger.snapshot(runtime_dir / 'arm.urdf') != runtime_snapshot:
            raise ValueError('Runtime URDF changed during generation; refusing to overwrite edits')
        publish(runtime_dir, staging)
    print(f'Published {report["total_triangles"]:,} independent collision faces / {report["total_bytes"] / 1024**2:.2f} MiB', flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets-dir', type=Path, default=Path(__file__).resolve().parents[1] / 'assets')
    generate(parser.parse_args().assets_dir)
