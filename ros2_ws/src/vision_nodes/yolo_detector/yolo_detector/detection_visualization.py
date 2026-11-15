"""在彩色图副本绘制实际发布的检测框，YOLO 与手动 ROI 使用同一路径。"""
import math

import cv2


def draw_detections(image, detections):
    output = image.copy()
    height, width = output.shape[:2]
    for detection in detections.detections:
        if not detection.results:
            continue
        best = max(detection.results, key=lambda item: item.hypothesis.score).hypothesis
        box = detection.bbox
        left = box.center.position.x - box.size_x / 2
        top = box.center.position.y - box.size_y / 2
        right = box.center.position.x + box.size_x / 2
        bottom = box.center.position.y + box.size_y / 2
        if (not all(math.isfinite(value) for value in (left, top, right, bottom, best.score))
                or not 0 <= best.score <= 1 or right <= left or bottom <= top
                or right < 0 or bottom < 0 or left >= width or top >= height):
            continue
        a = (round(max(0, min(left, width - 1))), round(max(0, min(top, height - 1))))
        b = (round(max(0, min(right, width - 1))), round(max(0, min(bottom, height - 1))))
        cv2.rectangle(output, a, b, (0, 255, 0), 2, cv2.LINE_AA)
        label = f'{best.class_id}: {best.score:.2f}'
        cv2.putText(output, label, (a[0], max(16, a[1] - 6)), cv2.FONT_HERSHEY_SIMPLEX,
                    0.55, (0, 255, 0), 1, cv2.LINE_AA)
    return output
