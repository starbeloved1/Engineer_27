#!/usr/bin/env python3
"""生成安装用模型，只转换网格路径，assets 保持为唯一模型来源"""
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET


def prepare(assets: Path, output: Path, urdf_name='arm.urdf', package_prefix=''):
    assets = assets.resolve()
    root = ET.parse(assets / urdf_name).getroot()
    count = 0
    for mesh in root.iter('mesh'):
        filename = mesh.get('filename', '')
        if '://' in filename or Path(filename).is_absolute():
            raise ValueError(f'Expected an assets-relative mesh path: {filename}')
        path = (assets / filename).resolve()
        if not path.is_file() or not path.is_relative_to(assets):
            raise ValueError(f'Missing or external mesh: {filename}')
        relative = path.relative_to(assets).as_posix()
        if not (relative.startswith('meshes/visual/') or relative.startswith('meshes/collision/')
                or (urdf_name == 'energy.urdf' and relative == 'energy.stl')):
            raise ValueError(f'Only runtime visual/collision meshes are exported: {relative}')
        mesh.set('filename', 'package://moveit_config/' +
                 (package_prefix.rstrip('/') + '/' if package_prefix else '') + relative)
        count += 1
    if count == 0:
        raise ValueError('No model mesh references')
    output.parent.mkdir(parents=True, exist_ok=True)
    ET.indent(root, space='  ')
    ET.ElementTree(root).write(output, encoding='utf-8', xml_declaration=True)
    print(f'Prepared {count} package mesh references: {output}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--urdf-name', default='arm.urdf')
    parser.add_argument('--package-prefix', default='')
    args = parser.parse_args()
    prepare(args.assets_dir, args.output, args.urdf_name, args.package_prefix)
