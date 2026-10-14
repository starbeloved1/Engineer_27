#!/usr/bin/env python3
"""Generate lightweight visual STL meshes from the immutable CAD-part archive.

Run with conda run -n engineer27-mesh python. Geometry is for visualization, not collision
checking. Sampled surface errors are diagnostics, not a Hausdorff guarantee.
"""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import time
import xml.etree.ElementTree as ET

import fast_simplification
import numpy as np
import trimesh

import merge_link_meshes as merger


BUDGETS = {"base_link": 300_000, "link_1": 40_000, "link_2": 45_000,
           "link_3": 35_000, "link_4": 30_000, "link_5": 15_000, "link_6": 35_000}
MAX_SAMPLE_ERROR = 0.002
P95_SAMPLE_ERROR = 0.0005
MAX_PART_BOUND_ERROR = 0.001
SAMPLE_COUNT = 768


def verify_first_merge(assets: Path) -> None:
    archive = assets / "source" / "merged_v1"
    manifest = json.loads((archive / "archive_manifest.json").read_text())["files"]
    for relative, expected in manifest.items():
        path = archive / relative
        if not path.is_file() or merger.snapshot(path) != expected:
            raise ValueError(f"First merged archive missing or changed: {path}")


def indexed_mesh(path: Path) -> trimesh.Trimesh:
    merger.triangle_count(path)
    records = np.fromfile(path, dtype=merger.TRIANGLE, offset=84)
    vertices, inverse = np.unique(records["vertices"].reshape(-1, 3), axis=0, return_inverse=True)
    faces = inverse.reshape(-1, 3)
    points = vertices[faces].astype(float)
    valid = np.linalg.norm(np.cross(points[:, 1] - points[:, 0], points[:, 2] - points[:, 0]), axis=1) > 0
    faces = faces[valid]
    if not len(faces) or not np.isfinite(vertices).all():
        raise ValueError(f"No finite surface triangles: {path}")
    return trimesh.Trimesh(vertices=vertices, faces=faces, process=False)


def sampled_distance(origin: trimesh.Trimesh, destination: trimesh.Trimesh) -> dict:
    points, _ = trimesh.sample.sample_surface(origin, SAMPLE_COUNT, seed=42)
    # Include axial extreme vertices so small protrusions are not solely area-sampled.
    extreme = np.r_[np.argmin(origin.vertices, axis=0), np.argmax(origin.vertices, axis=0)]
    points = np.vstack([points, origin.vertices[extreme]])
    distances = []
    for start in range(0, len(points), 128):
        _, distance, _ = trimesh.proximity.closest_point(destination, points[start:start + 128])
        distances.append(distance)
    values = np.concatenate(distances)
    return {"max_m": float(values.max()), "p95_m": float(np.percentile(values, 95)),
            "rms_m": float(np.sqrt(np.mean(values ** 2))), "samples": len(values)}


def assess(source: trimesh.Trimesh, candidate: trimesh.Trimesh) -> dict:
    forward = sampled_distance(source, candidate)
    reverse = sampled_distance(candidate, source)
    bound_error = float(np.max(np.abs(source.bounds - candidate.bounds)))
    accepted = (
        max(forward["max_m"], reverse["max_m"]) <= MAX_SAMPLE_ERROR
        and max(forward["p95_m"], reverse["p95_m"]) <= P95_SAMPLE_ERROR
        and bound_error <= MAX_PART_BOUND_ERROR
    )
    return {"source_to_output": forward, "output_to_source": reverse,
            "bounds_error_m": bound_error, "accepted": bool(accepted)}


def simplify_part(path: Path, requested: int) -> tuple[np.ndarray, dict]:
    source = indexed_mesh(path)
    count = len(source.faces)
    target = min(count, max(128, requested))
    attempts = []
    while target < count:
        vertices, faces = fast_simplification.simplify(
            np.asarray(source.vertices), np.asarray(source.faces),
            target_count=target, agg=5.0, preserve_border=True,
        )
        # Validate the actual float32 coordinates written to STL.
        candidate = trimesh.Trimesh(vertices=vertices.astype(np.float32), faces=faces, process=False)
        quality = assess(source, candidate)
        attempts.append({"requested_triangles": target, "triangles": len(faces), **quality})
        if quality["accepted"]:
            triangles = np.asarray(candidate.vertices[candidate.faces], dtype=np.float32)
            return triangles, {"input_triangles": merger.triangle_count(path),
                               "output_triangles": len(faces), "attempts": attempts, **quality}
        target = min(count, max(target + 1, target * 2))
    # Retain any part which cannot be simplified within the visual error budget.
    triangles = np.asarray(source.vertices[source.faces], dtype=np.float32)
    zero = {"max_m": 0.0, "p95_m": 0.0, "rms_m": 0.0, "samples": 0}
    return triangles, {"input_triangles": merger.triangle_count(path),
                       "output_triangles": count, "attempts": attempts,
                       "source_to_output": zero, "output_to_source": zero,
                       "bounds_error_m": 0.0, "accepted": True, "retained_full_surface": True}


def budgeted_part(path: Path, requested: int) -> tuple[np.ndarray, dict]:
    """Honor the face cap; surface distances are reported rather than hidden by retries."""
    source = indexed_mesh(path)
    candidate = source
    attempts = []
    if len(source.faces) > requested:
        for aggression in (7.0, 10.0, 15.0):
            vertices, faces = fast_simplification.simplify(
                source.vertices, source.faces, target_count=requested,
                agg=aggression, preserve_border=False,
            )
            attempts.append({"aggression": aggression, "triangles": len(faces)})
            if len(faces) and len(faces) <= requested:
                candidate = trimesh.Trimesh(vertices=vertices.astype(np.float32), faces=faces, process=False)
                break
        else:
            # CAD tessellations can have topology which blocks edge collapses.
            # Cluster nearby vertices without discarding arbitrary triangles.
            candidate = clustered_mesh(source, requested)
            attempts.append({"method": "vertex_clustering", "triangles": len(candidate.faces)})
    quality = assess(source, candidate) if candidate is not source else {
        "source_to_output": {"max_m": 0., "p95_m": 0., "rms_m": 0., "samples": 0},
        "output_to_source": {"max_m": 0., "p95_m": 0., "rms_m": 0., "samples": 0},
        "bounds_error_m": 0., "accepted": True,
    }
    if candidate is not source and (
        max(quality['source_to_output']['max_m'], quality['output_to_source']['max_m']) > .01
        or max(quality['source_to_output']['p95_m'], quality['output_to_source']['p95_m']) > .003
    ):
        alternative = clustered_mesh(source, requested)
        alternate_quality = assess(source, alternative)
        def score(q):
            return max(q['source_to_output']['max_m'], q['output_to_source']['max_m']) + 2 * max(
                q['source_to_output']['p95_m'], q['output_to_source']['p95_m']) + 4 * q['bounds_error_m']
        if score(alternate_quality) < score(quality):
            candidate, quality = alternative, alternate_quality
            attempts.append({'method': 'vertex_clustering_quality_repair', 'triangles': len(candidate.faces)})
    return candidate.triangles.astype(np.float32), {
        "input_triangles": merger.triangle_count(path), "output_triangles": len(candidate.faces),
        "requested_triangles": requested, "attempts": attempts, **quality,
    }


def clustered_mesh(source: trimesh.Trimesh, requested: int) -> trimesh.Trimesh:
    vertices = np.asarray(source.vertices)
    lower = vertices.min(axis=0)
    upper = vertices.max(axis=0)

    def cluster(cell):
        grid = np.floor((vertices - lower) / cell).astype(np.int64)
        _, inverse = np.unique(grid, axis=0, return_inverse=True)
        size = inverse.max() + 1
        counts = np.bincount(inverse, minlength=size)
        points = np.column_stack([np.bincount(inverse, weights=vertices[:, axis], minlength=size) / counts
                                  for axis in range(3)])
        for axis in range(3):
            points[inverse[np.argmin(vertices[:, axis])], axis] = lower[axis]
            points[inverse[np.argmax(vertices[:, axis])], axis] = upper[axis]
        faces = inverse[source.faces]
        valid = (faces[:, 0] != faces[:, 1]) & (faces[:, 1] != faces[:, 2]) & (faces[:, 0] != faces[:, 2])
        faces = faces[valid]
        _, index = np.unique(np.sort(faces, axis=1), axis=0, return_index=True)
        faces = faces[np.sort(index)]
        triangles = points[faces]
        area = np.linalg.norm(np.cross(triangles[:, 1] - triangles[:, 0], triangles[:, 2] - triangles[:, 0]), axis=1)
        return trimesh.Trimesh(points.astype(np.float32), faces[area > 0], process=False)

    lo, hi = 0., float(np.linalg.norm(upper - lower))
    best = None
    for _ in range(22):
        cell = (lo + hi) / 2
        candidate = cluster(cell)
        if len(candidate.faces) <= requested:
            hi = cell
            if len(candidate.faces):
                best = candidate
        else:
            lo = cell
    if best is None:
        raise ValueError("Vertex clustering eliminated the entire visual part")
    return best


def allocate_budget(specs: list, budget: int) -> dict[Path, int]:
    """Allocate per-instance faces while retaining small and end-effector parts."""
    counts, occurrences = {}, {}
    for path, count, *_ in specs:
        counts[path] = count
        occurrences[path] = occurrences.get(path, 0) + 1
    minimum = {p: min(n, 24) for p, n in counts.items()}
    # These small open/curved gripper structures are visually important.
    for p, n in counts.items():
        if any(token in p.name for token in ("柔性手爪", "触手", "U形法兰", "夹爪连接板", "电机防护", "CDTC_P1")):
            minimum[p] = n
    required = sum(minimum[p] * occurrences[p] for p in counts)
    if required > budget:
        raise ValueError(f"Face budget {budget} cannot retain important parts ({required})")
    lo, hi = 0., 1.
    for _ in range(50):
        ratio = (lo + hi) / 2
        size = sum(max(minimum[p], int(n * ratio)) * occurrences[p] for p, n in counts.items())
        if size <= budget:
            lo = ratio
        else:
            hi = ratio
    allocations = {p: max(minimum[p], int(n * lo)) for p, n in counts.items()}
    return allocations


def write_stl(path: Path, parts: list[np.ndarray], link: str) -> None:
    count = sum(len(part) for part in parts)
    with path.open("wb") as stream:
        stream.write(f"Engineer_27 lightweight {link}".encode("ascii").ljust(80, b"\0"))
        stream.write(struct.pack("<I", count))
        for part in parts:
            for start in range(0, len(part), 100_000):
                points = part[start:start + 100_000]
                records = np.zeros(len(points), dtype=merger.TRIANGLE)
                records["vertices"] = points
                points = points.astype(float)
                normal = np.cross(points[:, 1] - points[:, 0], points[:, 2] - points[:, 0])
                lengths = np.linalg.norm(normal, axis=1)
                np.divide(normal, lengths[:, None], out=normal, where=lengths[:, None] > 0)
                records["normal"] = normal
                records.tofile(stream)


def generate(assets: Path, strict_budget: bool = True) -> dict:
    assets = assets.resolve()
    merger.ensure_archive(assets)
    verify_first_merge(assets)
    source_path = assets / "source" / "arm.urdf"
    source = ET.parse(source_path).getroot()
    runtime = ET.parse(assets / "arm.urdf").getroot()
    runtime_snapshot = merger.snapshot(assets / "arm.urdf")
    original_runtime = copy.deepcopy(runtime)
    source_links = {link.get("name"): link for link in source.findall("link")}
    runtime_links = {link.get("name"): link for link in runtime.findall("link")}
    if set(source_links) != set(runtime_links) or set(source_links) != set(BUDGETS):
        raise ValueError("Unexpected link set; review the visual budgets before regeneration")
    cache = {}
    report = {"operation": "visual_simplification", "source": "source/arm.urdf",
              "first_merged_version": "source/merged_v1/arm.urdf",
              "initial_total_face_budget": sum(BUDGETS.values()),
              "strict_total_face_cap": strict_budget,
              "algorithm": "Per-part quadric decimation; important gripper parts retained" if strict_budget else "Per-part quadric decimation; preserve open borders; agg=5",
              "validation": {"sample_count_per_direction": SAMPLE_COUNT,
                             "max_sample_surface_error_m": MAX_SAMPLE_ERROR,
                             "p95_sample_surface_error_m": P95_SAMPLE_ERROR,
                             "max_part_bounds_error_m": MAX_PART_BOUND_ERROR,
                             "note": "Sampling is not an exhaustive surface-error guarantee. Visual only; do not use as collision geometry."},
              "links": {}, "parts": {}}
    start = time.monotonic()
    with tempfile.TemporaryDirectory(prefix=".simplify-", dir=assets) as temporary:
        staging = Path(temporary)
        destination = staging / "meshes" / "visual"
        destination.mkdir(parents=True)
        for name in ["link_5", "link_6", "link_4", "link_3", "link_2", "link_1", "base_link"]:
            specs = merger.visual_specs(source_links[name], source_path)
            source_count = sum(spec[1] for spec in specs)
            ratio = min(1.0, BUDGETS[name] / source_count)
            allocations = allocate_budget(specs, BUDGETS[name]) if strict_budget else None
            parts = []
            max_error = p95_error = 0.0
            for path, count, R, xyz, scale in specs:
                requested = allocations[path] if strict_budget else min(count, max(128, round(count * ratio)))
                key = (str(path), requested)
                if key not in cache:
                    triangles, quality = budgeted_part(path, requested) if strict_budget else simplify_part(path, requested)
                    cache[key] = (triangles, quality)
                    identity = f"{path.name}@{requested}"
                    report["parts"][identity] = quality
                    print(f"  {name}: {path.name}: {count:,} -> {len(triangles):,}", flush=True)
                triangles, quality = cache[key]
                part = ((triangles.astype(float) * scale) @ R.T + xyz).astype(np.float32)
                parts.append(part)
                max_error = max(max_error, quality["source_to_output"]["max_m"], quality["output_to_source"]["max_m"])
                p95_error = max(p95_error, quality["source_to_output"]["p95_m"], quality["output_to_source"]["p95_m"])
            output = destination / f"{name}.stl"
            write_stl(output, parts, name)
            count = merger.triangle_count(output)
            lo = np.minimum.reduce([part.min(axis=(0, 1)) for part in parts])
            hi = np.maximum.reduce([part.max(axis=(0, 1)) for part in parts])
            original_mesh = assets / "source" / "merged_v1" / "meshes" / "visual" / f"{name}.stl"
            original_records = np.memmap(original_mesh, dtype=merger.TRIANGLE, offset=84, mode="r")
            source_bounds = np.array([original_records["vertices"].min(axis=(0, 1)),
                                      original_records["vertices"].max(axis=(0, 1))])
            bound_error = float(np.max(np.abs(source_bounds - [lo, hi])))
            del original_records
            if bound_error > (0.005 if strict_budget else MAX_SAMPLE_ERROR):
                raise ValueError(f"Link bounds changed excessively: {name}: {bound_error}")
            report["links"][name] = {"visual_instances": len(specs), "source_triangles": source_count,
                                    "initial_budget": BUDGETS[name], "triangles": count,
                                    "reduction_fraction": 1 - count / source_count,
                                    "max_part_sample_surface_error_m": max_error,
                                    "max_part_p95_surface_error_m": p95_error,
                                    "source_bounds_m": source_bounds.tolist(), "output_bounds_m": [lo.tolist(), hi.tolist()],
                                    "bounds_error_m": bound_error, "bytes": output.stat().st_size}
            link = runtime_links[name]
            for visual in link.findall("visual"):
                link.remove(visual)
            visual = ET.SubElement(link, "visual", {"name": f"{name}_visual"})
            ET.SubElement(visual, "origin", {"xyz": "0 0 0", "rpy": "0 0 0"})
            geometry = ET.SubElement(visual, "geometry")
            ET.SubElement(geometry, "mesh", {"filename": f"meshes/visual/{name}.stl"})
            print(f"{name}: {source_count:,} -> {count:,} triangles; bounds delta {bound_error * 1000:.3f} mm", flush=True)
        if merger.nonvisual_tree(runtime) != merger.nonvisual_tree(original_runtime):
            raise ValueError("Non-visual runtime configuration changed")
        ET.indent(runtime, space="  ")
        urdf = staging / "arm.urdf"
        ET.ElementTree(runtime).write(urdf, encoding="utf-8", xml_declaration=True)
        checked = subprocess.run(["/opt/ros/humble/bin/check_urdf", str(urdf)], capture_output=True, text=True, timeout=30)
        if checked.returncode:
            raise ValueError(checked.stdout + checked.stderr)
        report["source_total_triangles"] = sum(v["source_triangles"] for v in report["links"].values())
        report["output_total_triangles"] = sum(v["triangles"] for v in report["links"].values())
        report["output_total_bytes"] = sum(v["bytes"] for v in report["links"].values())
        if strict_budget and report["output_total_triangles"] > sum(BUDGETS.values()):
            raise ValueError("Visual output exceeds the total face cap")
        report["validation"]["thresholds_enforced"] = not strict_budget
        report["validation"]["note"] = (
            "Distances are sampled diagnostics. In face-cap mode the previous 2 mm / 0.5 mm surface thresholds are not enforced. "
            "Visual only; independent collision geometry must come from original CAD-part meshes."
        )
        (staging / "mesh_merge_report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        merger.ensure_archive(assets)
        verify_first_merge(assets)
        if merger.snapshot(assets / 'arm.urdf') != runtime_snapshot:
            raise ValueError('Runtime URDF changed during generation; refusing to overwrite edits')
        merger.publish(assets, staging)
    print(f"Published lightweight visual: {report['output_total_triangles']:,} triangles, "
          f"{report['output_total_bytes'] / 1024**2:.2f} MiB; {time.monotonic() - start:.1f} seconds", flush=True)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assets-dir", type=Path, default=Path(__file__).resolve().parents[1] / "assets")
    parser.add_argument("--quality-priority", action="store_true", help="Permit extra faces to meet the previous sampled error thresholds")
    args = parser.parse_args()
    generate(args.assets_dir, strict_budget=not args.quality_priority)


if __name__ == "__main__":
    main()
