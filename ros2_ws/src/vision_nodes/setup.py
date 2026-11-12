from glob import glob

from setuptools import find_packages, setup


setup(
    name='vision_nodes',
    version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/vision_nodes']),
        ('share/vision_nodes', ['package.xml', 'README.md']),
        ('share/vision_nodes/launch', glob('launch/*.launch.py')),
        ('share/vision_nodes/config', glob('config/*.yaml')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Engineer 27 Team',
    maintainer_email='engineer27@example.com',
    description='D435 acquisition and unified visualization output for Foxglove.',
    license='TODO',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'main_visualizer = vision_nodes.main_visualizer_node:main',
        ],
    },
)
