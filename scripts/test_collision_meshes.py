"""Collision cover containment, concave openings and publication rollback."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import numpy as np
from scipy.spatial import ConvexHull
import trimesh

import build_collision_meshes as collision
import merge_link_meshes as merger
import simplify_visual_meshes as visual
import test_simplify_visual_meshes as visual_tests
import contextlib
import io
import xml.etree.ElementTree as ET


class CollisionTests(unittest.TestCase):
    def test_generation_preserves_visuals_archives_and_repeats(self):
        fixture = visual_tests.VisualTests()
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        before = collision.without_collision(ET.parse(fixture.assets / 'arm.urdf').getroot())
        archived = visual_tests.snapshots(fixture.assets / 'source')
        visuals = visual_tests.snapshots(fixture.assets / 'meshes/visual')
        with contextlib.redirect_stdout(io.StringIO()):
            collision.generate(fixture.assets)
        root = ET.parse(fixture.assets / 'arm.urdf').getroot()
        self.assertEqual(len(root.findall('.//collision')), 7)
        self.assertEqual(before, collision.without_collision(root))
        self.assertEqual(visuals, visual_tests.snapshots(fixture.assets / 'meshes/visual'))
        generated = visual_tests.snapshots(fixture.assets / 'meshes/collision')
        with contextlib.redirect_stdout(io.StringIO()):
            collision.generate(fixture.assets)
        self.assertEqual(generated, visual_tests.snapshots(fixture.assets / 'meshes/collision'))
        self.assertEqual(archived, visual_tests.snapshots(fixture.assets / 'source'))
        runtime = merger.snapshot(fixture.assets / 'arm.urdf')
        (fixture.assets / 'source/meshes/sphere.stl').unlink()
        with self.assertRaises(FileNotFoundError):
            collision.generate(fixture.assets)
        self.assertEqual(runtime, merger.snapshot(fixture.assets / 'arm.urdf'))
        self.assertEqual(generated, visual_tests.snapshots(fixture.assets / 'meshes/collision'))

    def test_box_and_cylinder_covers_contain_entire_source_faces(self):
        for source in (trimesh.creation.box(extents=[.1, .02, .03]),
                       trimesh.creation.cylinder(radius=.02, height=.08, sections=64)):
            source.apply_transform(trimesh.transformations.rotation_matrix(.4, [1, 2, 3]))
            source.apply_translation([.1, -.2, .3])
            cover, report = collision.convex_cover(source.triangles)
            hull = ConvexHull(cover.vertices)
            violation = source.vertices @ hull.equations[:, :3].T + hull.equations[:, 3]
            self.assertLessEqual(violation.max(), 1e-7)
            self.assertTrue(cover.is_watertight)
            self.assertGreater(cover.volume, 0)
            self.assertLessEqual(report['max_source_vertex_plane_violation_m'], 1e-7)

    def test_u_shaped_opening_is_preserved(self):
        source = trimesh.util.concatenate([
            trimesh.creation.box(extents=[.01, .08, .01], transform=trimesh.transformations.translation_matrix([-.035, 0, 0])),
            trimesh.creation.box(extents=[.01, .08, .01], transform=trimesh.transformations.translation_matrix([.035, 0, 0])),
            trimesh.creation.box(extents=[.08, .01, .01], transform=trimesh.transformations.translation_matrix([0, -.035, 0])),
        ])
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'u.stl'
            visual.write_stl(path, [source.triangles.astype(np.float32)], 'fixture')
            pieces, report = collision.build_part(path)
            self.assertTrue(report['all_source_triangles_covered'])
            probe = np.array([0., .025, 0.])
            for triangles in pieces:
                points, inverse = np.unique(triangles.reshape(-1, 3), axis=0, return_inverse=True)
                mesh = trimesh.Trimesh(points, inverse.reshape(-1, 3), process=False)
                self.assertFalse(mesh.contains([probe])[0])

    def test_collision_publication_failure_restores_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            assets, staging = root / 'assets', root / 'staging'
            for path, data in ((assets, b'old'), (staging, b'new')):
                (path / 'meshes/collision').mkdir(parents=True)
                (path / 'meshes/collision/base.stl').write_bytes(data)
                (path / 'arm.urdf').write_bytes(data)
                (path / 'collision_report.json').write_bytes(data)
            real_replace = collision.os.replace
            def fail(source, destination):
                if Path(source) == staging / 'collision_report.json':
                    raise OSError('simulated failure')
                return real_replace(source, destination)
            with patch.object(collision.os, 'replace', side_effect=fail), self.assertRaises(OSError):
                collision.publish(assets, staging)
            self.assertEqual((assets / 'arm.urdf').read_bytes(), b'old')
            self.assertEqual((assets / 'meshes/collision/base.stl').read_bytes(), b'old')
            self.assertEqual((assets / 'collision_report.json').read_bytes(), b'old')


if __name__ == '__main__':
    unittest.main()
