from glob import glob
from setuptools import find_packages, setup

setup(
    name='video_recorder', version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/video_recorder']),
        ('share/video_recorder', ['package.xml']),
        ('share/video_recorder/config', glob('config/*.yaml')),
        ('share/video_recorder/launch', glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'], zip_safe=True,
    maintainer='Engineer 27 Team', maintainer_email='engineer27@example.com',
    description='相机 RGB 原图 MP4 录像', license='MIT', tests_require=['pytest'],
    entry_points={'console_scripts': ['video_recorder_node = video_recorder.video_recorder_node:main']},
)
