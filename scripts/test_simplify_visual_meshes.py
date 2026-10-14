"""Exercise decimation, preserved archives and failed input handling on small meshes."""

import contextlib
import io
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

import numpy as np
import trimesh

import merge_link_meshes as merger
import simplify_visual_meshes as light


def snapshots(directory):
    return {str(p.relative_to(directory)): merger.snapshot(p)
            for p in directory.rglob("*") if p.is_file()}


class VisualTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.assets = Path(self.temporary.name) / "assets"
        meshes = self.assets / "meshes"
        meshes.mkdir(parents=True)
        sphere = trimesh.creation.icosphere(subdivisions=3, radius=0.04)
        light.write_stl(meshes / "sphere.stl", [sphere.triangles.astype(np.float32)], "fixture")
        root = ET.Element("robot", {"name": "fixture"})
        names = ["base_link", *[f"link_{i}" for i in range(1, 7)]]
        for i, name in enumerate(names):
            link = ET.SubElement(root, "link", {"name": name})
            visual = ET.SubElement(link, "visual")
            ET.SubElement(visual, "origin", {"xyz": f"{i * 0.1} 0 0", "rpy": "0 0 0.3"})
            ET.SubElement(ET.SubElement(visual, "geometry"), "mesh", {"filename": "meshes/sphere.stl"})
            ET.SubElement(ET.SubElement(ET.SubElement(link, "collision"), "geometry"), "box", {"size": "0.1 0.1 0.1"})
            if i:
                joint = ET.SubElement(root, "joint", {"name": f"joint_{i}", "type": "revolute"})
                ET.SubElement(joint, "parent", {"link": names[i - 1]})
                ET.SubElement(joint, "child", {"link": name})
                ET.SubElement(joint, "axis", {"xyz": "0 1 0"})
                ET.SubElement(joint, "limit", {"lower": "-1", "upper": "1", "effort": "2", "velocity": "1"})
        ET.ElementTree(root).write(self.assets / "arm.urdf")
        with contextlib.redirect_stdout(io.StringIO()):
            merger.run(self.assets)
        first = self.assets / "source" / "merged_v1"
        first.mkdir()
        for relative in ("arm.urdf", "mesh_merge_report.json", "meshes/visual"):
            source = self.assets / relative
            target = first / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            if source.is_dir():
                shutil.copytree(source, target)
            else:
                shutil.copy2(source, target)
        (first / "archive_manifest.json").write_text(json.dumps({"files": snapshots(first)}))
        self.budgets = {name: 128 for name in names}

    def generate(self, strict_budget=False):
        with patch.object(light, "BUDGETS", self.budgets), contextlib.redirect_stdout(io.StringIO()):
            return light.generate(self.assets, strict_budget=strict_budget)

    def test_reduction_positions_configuration_and_repeatability(self):
        archived = snapshots(self.assets / "source")
        nonvisual = merger.nonvisual_tree(ET.parse(self.assets / "arm.urdf").getroot())
        report = self.generate()
        self.assertLess(report["output_total_triangles"], report["source_total_triangles"])
        root = ET.parse(self.assets / "arm.urdf").getroot()
        self.assertEqual(nonvisual, merger.nonvisual_tree(root))
        self.assertEqual(len(root.findall(".//visual")), 7)
        for info in report["links"].values():
            self.assertLessEqual(info["bounds_error_m"], light.MAX_SAMPLE_ERROR)
            self.assertLessEqual(info["max_part_sample_surface_error_m"], light.MAX_SAMPLE_ERROR)
        before = snapshots(self.assets / "meshes" / "visual")
        self.generate()
        self.assertEqual(before, snapshots(self.assets / "meshes" / "visual"))
        self.assertEqual(archived, snapshots(self.assets / "source"))

    def test_missing_source_keeps_current_runtime(self):
        before = snapshots(self.assets / "meshes" / "visual")
        urdf = merger.snapshot(self.assets / "arm.urdf")
        (self.assets / "source" / "meshes" / "sphere.stl").unlink()
        with self.assertRaises(FileNotFoundError):
            self.generate()
        self.assertEqual(before, snapshots(self.assets / "meshes" / "visual"))
        self.assertEqual(urdf, merger.snapshot(self.assets / "arm.urdf"))

    def test_face_cap_is_enforced_and_existing_collision_is_preserved(self):
        original = merger.nonvisual_tree(ET.parse(self.assets / "arm.urdf").getroot())
        report = self.generate(strict_budget=True)
        self.assertLessEqual(report['output_total_triangles'], sum(self.budgets.values()))
        self.assertFalse(report['validation']['thresholds_enforced'])
        self.assertEqual(original, merger.nonvisual_tree(ET.parse(self.assets / 'arm.urdf').getroot()))
        for name, link in report['links'].items():
            self.assertLessEqual(link['triangles'], self.budgets[name])

    def test_changed_first_version_keeps_current_runtime(self):
        before = snapshots(self.assets / "meshes" / "visual")
        first = self.assets / "source" / "merged_v1" / "arm.urdf"
        first.write_bytes(first.read_bytes() + b"\n")
        with self.assertRaisesRegex(ValueError, "First merged archive"):
            self.generate()
        self.assertEqual(before, snapshots(self.assets / "meshes" / "visual"))


if __name__ == "__main__":
    unittest.main()
