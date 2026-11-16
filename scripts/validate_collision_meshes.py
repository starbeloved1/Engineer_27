#!/usr/bin/env python3
"""Compare FCL queries against full-resolution source meshes in 32 poses."""
import argparse
import json
from pathlib import Path
import time
import xml.etree.ElementTree as ET

import fcl
import numpy as np

import merge_link_meshes as merger


def load_object(path):
    records = np.fromfile(path, dtype=merger.TRIANGLE, offset=84)
    vertices = np.asarray(records['vertices'].reshape(-1, 3), dtype=np.float64)
    faces = np.arange(len(vertices), dtype=np.int32).reshape(-1, 3)
    model = fcl.BVHModel()
    model.beginModel(len(vertices), len(faces))
    model.addSubModel(vertices, faces)
    model.endModel()
    return fcl.CollisionObject(model)


def transforms(root, angles):
    joints = {j.find('child').get('link'): j for j in root.findall('joint')}
    result = {}
    def visit(name):
        if name in result:
            return result[name]
        if name not in joints:
            result[name] = np.eye(4)
            return result[name]
        joint = joints[name]
        origin = joint.find('origin')
        T = np.eye(4)
        T[:3, :3] = merger.rotation(merger.vector(origin.get('rpy') if origin is not None else None, '0 0 0'))
        T[:3, 3] = merger.vector(origin.get('xyz') if origin is not None else None, '0 0 0')
        axis = merger.vector(joint.find('axis').get('xyz'), '1 0 0')
        axis /= np.linalg.norm(axis)
        x, y, z = axis
        K = np.array([[0, -z, y], [z, 0, -x], [-y, x, 0]])
        a = angles.get(joint.get('name'), 0.)
        motion = np.eye(4)
        motion[:3, :3] = np.eye(3) + np.sin(a) * K + (1 - np.cos(a)) * K @ K
        result[name] = visit(joint.find('parent').get('link')) @ T @ motion
        return result[name]
    for link in root.findall('link'):
        visit(link.get('name'))
    return result


def pairs(objects, ignored):
    names = list(objects)
    collided = set()
    request = fcl.CollisionRequest(num_max_contacts=1)
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            pair = tuple(sorted((a, b)))
            if pair in ignored:
                continue
            if fcl.collide(objects[a], objects[b], request, fcl.CollisionResult()):
                collided.add(pair)
    return collided


def validate(assets):
    runtime = merger.runtime_directory(assets)
    root = ET.parse(runtime / 'arm.urdf').getroot()
    names = [l.get('name') for l in root.findall('link')]
    collision = {name: load_object(runtime / 'meshes/collision' / f'{name}.stl') for name in names}
    original = {name: load_object(assets / 'source/merged_v1/meshes/visual' / f'{name}.stl') for name in names}
    ignored = {tuple(sorted((j.find('parent').get('link'), j.find('child').get('link')))) for j in root.findall('joint')}
    joints = [j.get('name') for j in root.findall('joint')]
    poses = [[0.] * 6] + [[.1 if i == j else 0. for j in range(6)] for i in range(6)] + [[.2, .3, -.4, .5, .6, .7]]
    poses.extend(np.random.default_rng(42).uniform(-np.pi, np.pi, size=(24, 6)).tolist())
    result = {'engine': 'python-fcl 0.7.0.11', 'poses': [], 'adjacent_pairs_excluded_for_comparison_only': sorted(ignored),
              'note': 'No SRDF exclusions are installed. Random angles are offline geometric tests, not mechanically approved motions. Query differences are measured against the CAD triangle-surface model, not physical ground truth.'}
    for pose in poses:
        T = transforms(root, dict(zip(joints, pose)))
        for collection in (original, collision):
            for name, obj in collection.items():
                obj.setTransform(fcl.Transform(T[name][:3, :3], T[name][:3, 3]))
        start = time.perf_counter()
        reference = pairs(original, ignored)
        reference_seconds = time.perf_counter() - start
        start = time.perf_counter()
        candidate = pairs(collision, ignored)
        collision_seconds = time.perf_counter() - start
        info = {'joint_positions_rad': dict(zip(joints, pose)), 'source_colliding_pairs': sorted(reference),
                'collision_model_pairs': sorted(candidate), 'additional_pairs': sorted(candidate - reference),
                'missing_pairs': sorted(reference - candidate), 'source_query_seconds': reference_seconds,
                'collision_query_seconds': collision_seconds}
        result['poses'].append(info)
        print(json.dumps(info), flush=True)
    # Positive tests at exposed axis-extreme CAD points, and a remote negative probe.
    identity = fcl.Transform(np.eye(3), np.zeros(3))
    contact_tests = []
    for name in names:
        original[name].setTransform(identity)
        collision[name].setTransform(identity)
        mesh = np.memmap(assets / 'source/merged_v1/meshes/visual' / f'{name}.stl', dtype=merger.TRIANGLE, offset=84, mode='r')
        points = mesh['vertices'].reshape(-1, 3)
        extreme = np.r_[np.argmin(points, axis=0), np.argmax(points, axis=0)]
        for point in points[extreme]:
            probe = fcl.CollisionObject(fcl.Box(.008, .008, .008), fcl.Transform(point.astype(float)))
            positive = bool(fcl.collide(collision[name], probe, fcl.CollisionRequest(), fcl.CollisionResult()))
            contact_tests.append({'link': name, 'probe_center_m': point.tolist(), 'collision': positive})
        remote = fcl.CollisionObject(fcl.Box(.008, .008, .008), fcl.Transform(np.array([10., 10., 10.])))
        if fcl.collide(collision[name], remote, fcl.CollisionRequest(), fcl.CollisionResult()):
            raise ValueError(f'Remote probe incorrectly collides: {name}')
    result['exposed_surface_contact_tests'] = contact_tests
    result['remote_negative_tests'] = len(names)
    result['all_exposed_contact_tests_passed'] = all(t['collision'] for t in contact_tests)
    result['missing_pair_count'] = sum(len(p['missing_pairs']) for p in result['poses'])
    result['additional_pair_count'] = sum(len(p['additional_pairs']) for p in result['poses'])
    (runtime / 'collision_validation.json').write_text(json.dumps(result, indent=2) + '\n')
    if not result['all_exposed_contact_tests_passed'] or result['missing_pair_count']:
        raise ValueError('Collision validation found missed contacts; inspect collision_validation.json')
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets-dir', type=Path, default=Path(__file__).resolve().parents[1] / 'assets')
    validate(parser.parse_args().assets_dir.resolve())
