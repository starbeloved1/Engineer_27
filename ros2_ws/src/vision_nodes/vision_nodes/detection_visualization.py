"""检测框绘制，不依赖 ROS 或 YOLO"""
import math

import cv2


def draw_detections(image, boxes):
    """在 BGR 原图副本上绘制 (x1, y1, x2, y2, class_id, score)"""
    overlay = image.copy()
    height, width = overlay.shape[:2]
    for x1, y1, x2, y2, class_id, score in boxes:
        if (not all(math.isfinite(value) for value in (x1, y1, x2, y2, score))
                or x2 <= x1 or y2 <= y1 or not 0 <= score <= 1):
            continue
        if x2 < 0 or y2 < 0 or x1 >= width or y1 >= height:
            continue
        left, right = max(0, round(x1)), min(width - 1, round(x2))
        top, bottom = max(0, round(y1)), min(height - 1, round(y2))
        cv2.rectangle(overlay, (left, top), (right, bottom), (0, 255, 0), 2, cv2.LINE_AA)
        cv2.putText(overlay, f"{class_id}: {score:.2f}", (left, max(16, top - 6)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 1, cv2.LINE_AA)
    return overlay
