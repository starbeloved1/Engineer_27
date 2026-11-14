"""框内筛选、模型采样与局部 ICP；坐标、距离统一为米。"""
from pathlib import Path

import cv2
import numpy as np
import open3d as o3d
from scipy.spatial.transform import Rotation


def load_model(path, scale, sample_points, voxel_size):
    path = Path(path).expanduser().resolve()
    if not path.is_file():
        raise FileNotFoundError(f"矿模型不存在: {path}")
    mesh = o3d.io.read_triangle_mesh(str(path))
    if mesh.is_empty() or not mesh.has_triangles():
        raise ValueError(f"模型不是有效三角网格: {path}")
    # 保留原模型原点和轴向；先转换单位，再按米做体素下采样。
    mesh.scale(scale, center=np.zeros(3))
    if not np.isfinite(np.asarray(mesh.vertices)).all():
        raise ValueError("模型包含非有限坐标")
    o3d.utility.random.seed(27)
    cloud = mesh.sample_points_uniformly(number_of_points=sample_points)
    return cloud.voxel_down_sample(voxel_size) if voxel_size > 0 else cloud


def roi_mask(xyz, camera_matrix, distortion, cloud_to_color, bbox,
             image_size, min_depth, max_depth, shrink_ratio):
    """先将点变换到彩色光学坐标系，再投影到 YOLO 原图像素。"""
    mask = np.zeros(len(xyz), dtype=bool)
    cx, cy, width, height, theta = bbox
    if (not np.isfinite(bbox).all() or width <= 0 or height <= 0
            or abs(theta) > 1e-6):
        return mask
    width *= 1 - shrink_ratio
    height *= 1 - shrink_ratio
    x1, x2 = max(0, cx - width / 2), min(image_size[0], cx + width / 2)
    y1, y2 = max(0, cy - height / 2), min(image_size[1], cy + height / 2)
    if x2 <= x1 or y2 <= y1:
        return mask
    valid = np.isfinite(xyz).all(axis=1)
    indices = np.flatnonzero(valid)
    points = xyz[valid] @ cloud_to_color[:3, :3].T + cloud_to_color[:3, 3]
    depth_valid = (points[:, 2] >= min_depth) & (points[:, 2] <= max_depth)
    indices, points = indices[depth_valid], points[depth_valid]
    if not len(points):
        return mask
    pixels, _ = cv2.projectPoints(
        np.ascontiguousarray(points, dtype=np.float64), np.zeros(3), np.zeros(3),
        camera_matrix, distortion if len(distortion) else None)
    pixels = pixels.reshape(-1, 2)
    inside = (np.isfinite(pixels).all(axis=1)
              & (pixels[:, 0] >= x1) & (pixels[:, 0] < x2)
              & (pixels[:, 1] >= y1) & (pixels[:, 1] < y2))
    mask[indices[inside]] = True
    return mask


def filtered_cloud(xyz, rgb, voxel_size, sor_neighbors, sor_std_ratio, statistics=None):
    cloud = o3d.geometry.PointCloud(o3d.utility.Vector3dVector(xyz))
    counts = {'raw': len(cloud.points)}
    if rgb is not None:
        cloud.colors = o3d.utility.Vector3dVector(rgb.astype(np.float64) / 255)
    if voxel_size > 0:
        cloud = cloud.voxel_down_sample(voxel_size)
    counts['voxel'] = len(cloud.points)
    if sor_neighbors > 0 and len(cloud.points) > sor_neighbors:
        cloud, _ = cloud.remove_statistical_outlier(sor_neighbors, sor_std_ratio)
    counts['sor'] = len(cloud.points)
    if statistics is not None:
        statistics.update(counts)
    return cloud


def register_model(source, target, initial_rpy_deg, max_corr_dist, max_iterations):
    # 与 26 的 Python 方案一样，以质心差初始化平移；朝向为用户可调初值。
    # 这是局部配准，不把上帧第 i 个框当作稳定的目标身份。
    initial = np.eye(4)
    initial[:3, :3] = Rotation.from_euler('xyz', initial_rpy_deg, degrees=True).as_matrix()
    initial[:3, 3] = target.get_center() - initial[:3, :3] @ source.get_center()
    return o3d.pipelines.registration.registration_icp(
        source, target, max_corr_dist, initial,
        o3d.pipelines.registration.TransformationEstimationPointToPoint(),
        o3d.pipelines.registration.ICPConvergenceCriteria(max_iteration=max_iterations))


def accepted_registration(result, min_fitness, max_rmse, min_correspondences):
    return (np.isfinite(result.transformation).all()
            and np.isfinite(result.inlier_rmse)
            and result.fitness >= min_fitness
            and result.inlier_rmse <= max_rmse
            and len(result.correspondence_set) >= min_correspondences)
