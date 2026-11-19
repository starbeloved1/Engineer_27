from glob import glob
from os.path import relpath
from pathlib import Path
from setuptools import find_packages, setup

# colcon 的 symlink-install 会在 build 目录执行，先解析回源码位置。
model_file = Path(__file__).resolve().parents[4] / 'model' / 'best1.pt'

setup(
    name='yolo_detector', version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/yolo_detector']),
        ('share/yolo_detector', ['package.xml']),
        ('share/yolo_detector/config', glob('config/*.yaml')),
        ('share/yolo_detector/launch', glob('launch/*.launch.py')),
        ('share/yolo_detector', ['requirements.txt']),
        ('share/yolo_detector/model', [relpath(model_file)]),
    ],
    install_requires=['setuptools'], zip_safe=True,
    maintainer='Engineer 27 Team', maintainer_email='engineer27@example.com',
    description='YOLO、固定手动 ROI 与同帧叠框图像', license='MIT', tests_require=['pytest'],
    entry_points={'console_scripts': ['yolo_detector_node = yolo_detector.yolo_detector_node:main']},
)
