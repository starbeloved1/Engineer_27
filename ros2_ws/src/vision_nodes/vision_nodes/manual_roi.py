"""手动 ROI 的鼠标交互。只提交完成的框，不阻塞 ROS 图像回调。"""
import os

import cv2


class ManualRoiSelector:
    WINDOW = 'Manual ROI - drag to set or replace'

    def __init__(self):
        self.roi = None  # 原图像素 (x1, y1, x2, y2)，完成下一次框选前一直保持
        self.image = None
        self.source = None
        self.drag_start = None
        self.drag_end = None
        self.opened = False
        self.hidden = False

    def update_image(self, image, frame_id):
        source = (image.shape[1], image.shape[0], frame_id)
        changed = self.source is not None and source != self.source
        if changed:
            # 旧像素坐标不适用于另一幅尺寸/坐标系的图，禁止默默缩放或沿用。
            self.roi = None
            self.cancel_drag()
            self.close()
            self.hidden = False
        self.source = source
        self.image = image
        return changed

    def cancel_drag(self):
        self.drag_start = self.drag_end = None

    def mouse_callback(self, event, x, y, flags, userdata):
        if self.source is None:
            return
        width, height, _ = self.source
        point = (max(0, min(width - 1, x)), max(0, min(height - 1, y)))
        if event == cv2.EVENT_LBUTTONDOWN:
            self.drag_start = self.drag_end = point
        elif event == cv2.EVENT_MOUSEMOVE and self.drag_start is not None:
            self.drag_end = point
        elif event == cv2.EVENT_LBUTTONUP and self.drag_start is not None:
            x1, x2 = sorted((self.drag_start[0], point[0]))
            y1, y2 = sorted((self.drag_start[1], point[1]))
            if x2 - x1 >= 2 and y2 - y1 >= 2:
                self.roi = (x1, y1, x2, y2)
            self.cancel_drag()  # 点击或零面积框不覆盖上一个有效 ROI。

    def reopen(self):
        self.hidden = False
        self.cancel_drag()

    def close(self):
        if self.opened:
            try:
                cv2.destroyWindow(self.WINDOW)
            except cv2.error:
                pass
        self.opened = False
        self.cancel_drag()

    def poll(self):
        """由 ROS timer 调用，界面持续响应，选框期间仍正常处理每帧。"""
        if self.image is None or self.hidden:
            return
        if not self.opened:
            if not (os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')):
                raise RuntimeError('手动 ROI 需要桌面显示；请在本机图形终端启动或关闭 use_manual_roi')
            # 固定为原图尺寸，鼠标坐标就是原图像素，避免窗口缩放导致坐标错位。
            cv2.namedWindow(self.WINDOW, cv2.WINDOW_AUTOSIZE)
            cv2.setMouseCallback(self.WINDOW, self.mouse_callback)
            self.opened = True
        else:
            try:
                visible = cv2.getWindowProperty(self.WINDOW, cv2.WND_PROP_VISIBLE) >= 1
            except cv2.error:
                visible = False
            if not visible:
                self.opened = False
                self.hidden = True
                self.cancel_drag()
                return

        preview = self.image.copy()
        if self.roi is not None:
            x1, y1, x2, y2 = self.roi
            cv2.rectangle(preview, (x1, y1), (x2, y2), (0, 255, 0), 2)
        if self.drag_start is not None:
            cv2.rectangle(preview, self.drag_start, self.drag_end, (0, 255, 255), 1)
        label = 'Drag: set/replace | Esc: cancel drag | Q: hide'
        cv2.putText(preview, label, (8, 22), cv2.FONT_HERSHEY_SIMPLEX,
                    0.45, (0, 255, 255), 1, cv2.LINE_AA)
        cv2.imshow(self.WINDOW, preview)
        key = cv2.waitKey(1) & 0xff
        if key == 27:
            self.cancel_drag()
        elif key in (ord('q'), ord('Q')):
            self.close()
            self.hidden = True  # 隐藏交互窗口不清空 ROI，也不停止发布。
