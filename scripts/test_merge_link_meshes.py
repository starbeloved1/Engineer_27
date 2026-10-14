"""Meaningful geometry, archive and rollback checks for the STL converter."""

import contextlib
import hashlib
import io
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

import numpy as np

import merge_link_meshes as merger


def fingerprints(directory):
    paths = [directory / "arm.urdf", directory / "mesh_merge_report.json"]
    paths += sorted((directory / "meshes" / "visual").glob("*.stl"))
    return {str(p.relative_to(directory)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in paths if p.is_file()}


class MergeTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.assets = Path(self.temporary.name) / "assets"
        meshes = self.assets / "meshes"
        meshes.mkdir(parents=True)
        self.vertices = np.array([
            [[0, 0, 0], [1, 0, 0], [0, 1, 1]],
            [[0, 0, 0], [0, 0, 0], [0, 0, 0]],
        ], dtype=np.float32)
        records = np.zeros(2, dtype=merger.TRIANGLE)
        records["vertices"] = self.vertices
        records["attribute"] = [7, 12]
        with (meshes / "零件.stl").open("wb") as stream:
            stream.write(b"fixture".ljust(80, b"\0") + struct.pack("<I", 2))
            records.tofile(stream)
        # Keep an unreferenced mesh to verify that archiving is complete.
        (meshes / "unused.stl").write_bytes((meshes / "零件.stl").read_bytes())
        self.original_xml = b'''<?xml version="1.0"?>
<robot name="fixture">
  <link name="base">
    <visual>
      <origin xyz="0.3 -0.2 0.4" rpy="0 0 1.5707963267948966"/>
      <geometry><mesh filename="meshes/\xe9\x9b\xb6\xe4\xbb\xb6.stl" scale="1 2 1"/></geometry>
    </visual>
    <visual>
      <origin xyz="-1 0 0" rpy="0 0 0"/>
      <geometry><mesh filename="meshes/\xe9\x9b\xb6\xe4\xbb\xb6.stl"/></geometry>
    </visual>
  </link>
  <link name="tip">
    <inertial><origin xyz="0 0 0"/><mass value="1"/>
      <inertia ixx="1" ixy="0" ixz="0" iyy="1" iyz="0" izz="1"/></inertial>
    <visual><geometry><mesh filename="meshes/\xe9\x9b\xb6\xe4\xbb\xb6.stl"/></geometry></visual>
  </link>
  <joint name="hinge" type="revolute">
    <parent link="base"/><child link="tip"/>
    <origin xyz="0.1 0.2 0.3" rpy="0.2 -0.3 0.4"/>
    <axis xyz="0 1 0"/><limit lower="-1" upper="1" effort="5" velocity="0.5"/>
  </joint>
</robot>'''
        (self.assets / "arm.urdf").write_bytes(self.original_xml)

    def run_merge(self, chunk_size=1):
        with contextlib.redirect_stdout(io.StringIO()):
            return merger.run(self.assets, chunk_size)

    def test_geometry_archive_and_robot_configuration(self):
        report = self.run_merge()
        archive = self.assets / "source"
        self.assertEqual((archive / "arm.urdf").read_bytes(), self.original_xml)
        self.assertTrue((archive / "meshes" / "unused.stl").is_file())
        self.assertFalse((self.assets / "meshes" / "unused.stl").exists())
        self.assertEqual(report["base"]["triangles"], 4)
        self.assertEqual(report["base"]["degenerate_triangles"], 2)
        actual = np.fromfile(self.assets / "meshes" / "visual" / "base.stl",
                             dtype=merger.TRIANGLE, offset=84)
        # Independently derived for a +90 degree Z rotation, scale (1,2,1).
        expected = self.vertices.astype(float).copy()
        expected[..., 0] = -2 * self.vertices[..., 1] + 0.3
        expected[..., 1] = self.vertices[..., 0] - 0.2
        expected[..., 2] = self.vertices[..., 2] + 0.4
        second = self.vertices.astype(float) + [-1, 0, 0]
        np.testing.assert_allclose(actual["vertices"], np.concatenate([expected, second]),
                                   rtol=0, atol=merger.TOLERANCE)
        np.testing.assert_array_equal(actual["attribute"], [7, 12, 7, 12])
        original = ET.fromstring(self.original_xml)
        generated = ET.parse(self.assets / "arm.urdf").getroot()
        self.assertEqual(merger.nonvisual_tree(original), merger.nonvisual_tree(generated))
        self.assertEqual(len(generated.findall(".//visual")), 2)
        self.assertEqual(len(list((self.assets / "meshes" / "visual").glob("*.stl"))), 2)

    def test_repeated_generation_is_identical_and_archive_unchanged(self):
        self.run_merge()
        before = fingerprints(self.assets)
        archived = {str(p): merger.sha256(p) for p in (self.assets / "source").rglob("*") if p.is_file()}
        self.run_merge(chunk_size=100)
        self.assertEqual(fingerprints(self.assets), before)
        self.assertEqual({str(p): merger.sha256(p) for p in (self.assets / "source").rglob("*") if p.is_file()}, archived)

    def test_missing_archived_mesh_does_not_replace_runtime(self):
        self.run_merge()
        before = fingerprints(self.assets)
        (self.assets / "source" / "meshes" / "零件.stl").unlink()
        with self.assertRaises(FileNotFoundError):
            self.run_merge()
        self.assertEqual(fingerprints(self.assets), before)

    def test_changed_archive_is_rejected(self):
        self.run_merge()
        before = fingerprints(self.assets)
        path = self.assets / "source" / "arm.urdf"
        path.write_bytes(path.read_bytes() + b"\n")
        with self.assertRaisesRegex(ValueError, "Archived input changed"):
            self.run_merge()
        self.assertEqual(fingerprints(self.assets), before)

    def test_separate_regeneration_preserves_runtime_and_archive(self):
        self.run_merge()
        before = fingerprints(self.assets)
        archived = merger.snapshot(self.assets / "source" / "arm.urdf")
        destination = self.assets / "source" / "merged_regenerated"
        with contextlib.redirect_stdout(io.StringIO()):
            merger.run(self.assets, output_dir=destination)
        self.assertEqual(fingerprints(self.assets), before)
        self.assertEqual(merger.snapshot(self.assets / "source" / "arm.urdf"), archived)
        self.assertEqual(merger.triangle_count(destination / "meshes" / "visual" / "base.stl"), 4)

    def test_archived_output_destinations_are_rejected(self):
        self.run_merge()
        before = fingerprints(self.assets)
        for relative in ("source", "source/merged_v1", "source/meshes", "source/merged_v1/nested"):
            with self.assertRaisesRegex(ValueError, "Cannot overwrite archived inputs"):
                merger.run(self.assets, output_dir=self.assets / relative)
        self.assertEqual(fingerprints(self.assets), before)

    def test_missing_reference_keeps_original_runtime_and_meshes(self):
        broken = self.original_xml.replace(b"meshes/", b"missing/")
        (self.assets / "arm.urdf").write_bytes(broken)
        with self.assertRaises(FileNotFoundError):
            self.run_merge()
        self.assertEqual((self.assets / "arm.urdf").read_bytes(), broken)
        self.assertTrue((self.assets / "meshes" / "零件.stl").is_file())

    def test_publication_failure_restores_previous_outputs(self):
        self.run_merge()
        before = fingerprints(self.assets)
        real_replace = merger.os.replace

        def fail_on_report(source, target):
            source, target = Path(source), Path(target)
            if (target == self.assets / "mesh_merge_report.json"
                    and source.parent.name.startswith(".merge-")):
                raise OSError("simulated install failure")
            return real_replace(source, target)

        with patch.object(merger.os, "replace", side_effect=fail_on_report):
            with self.assertRaisesRegex(OSError, "simulated install failure"):
                self.run_merge()
        self.assertEqual(fingerprints(self.assets), before)


if __name__ == "__main__":
    unittest.main()
