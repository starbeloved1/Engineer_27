"""将 JSON 启动配置和命令行覆盖转换为 run.sh 的启动参数"""

import argparse
from copy import deepcopy
import hashlib
import json
import math
import os
from pathlib import Path
import sys
import tempfile

import yaml


BOOLEAN_OPTIONS = {
    "debug", "use_rviz", "start_camera", "start_detector", "start_icp", "start_bridge",
}
PATH_OPTIONS = {
    "hardware_config", "controller_config", "sim_config", "mtc_config",
    "camera_config", "vision_config", "foxglove_config", "yolo_python",
}
OPTIONS = BOOLEAN_OPTIONS | PATH_OPTIONS | {"mode"}
PROJECT_ROOT = Path(__file__).resolve().parents[2]
PARAMETER_FILES = {
    "mtc_config": ["ros2_ws/src/engineer_mtc/config/engineer_mtc.yaml"],
    "hardware_config": ["ros2_ws/src/serial_driver/config/serial_driver.yaml"],
    "camera_config": ["ros2_ws/src/vision_nodes/camera_driver/config/camera.yaml"],
    "vision_config": [
        "ros2_ws/src/vision_nodes/yolo_detector/config/yolo_detector.yaml",
        "ros2_ws/src/vision_nodes/icp_registration/config/icp_registration.yaml",
        "ros2_ws/src/vision_nodes/cloud_publisher/config/model_cloud_publisher.yaml",
    ],
}
PARAMETER_GROUPS = {
    "engineer_mtc": ("mtc_config", ("engineer_mtc", "ros__parameters")),
    "serial_driver": ("hardware_config", ()),
    "video_recorder": ("camera_config", ("/video_recorder", "ros__parameters")),
    "yolo_detector": ("vision_config", ("yolo_detector", "ros__parameters")),
    "icp_registration": ("vision_config", ("icp_registration", "ros__parameters")),
}


def merge_config(base, override):
    """递归合并 YAML 字段，后者覆盖前者"""
    for key, value in override.items():
        if isinstance(value, dict) and isinstance(base.get(key), dict):
            merge_config(base[key], value)
        else:
            base[key] = deepcopy(value)


def parameter_value(value, reference, name):
    """校验参数类型，并将 JSON 整数转换为 YAML 所需的浮点类型"""
    if type(reference) is float and type(value) in (int, float):
        if not math.isfinite(value):
            raise ValueError(f"{name} 必须为有限数值")
        return float(value)
    if type(value) is not type(reference):
        raise ValueError(f"{name} 的类型须与包内 YAML 一致")
    if isinstance(value, list) and reference:
        return [parameter_value(item, reference[0], name) for item in value]
    return value


def apply_parameter_overrides(arguments, parameters):
    """将常调参数合并为完整配置，写入独立缓存文件并更新启动路径"""
    configs = {}
    for node, overrides in parameters.items():
        if node not in PARAMETER_GROUPS or not isinstance(overrides, dict):
            raise ValueError(f"parameters.{node} 必须为已支持节点的参数对象")
        if not overrides:
            continue
        option, parameter_path = PARAMETER_GROUPS[node]
        if option not in configs:
            config = {}
            for filename in PARAMETER_FILES[option]:
                with (PROJECT_ROOT / filename).open(encoding="utf-8") as stream:
                    defaults = yaml.safe_load(stream)
                if not isinstance(defaults, dict):
                    raise ValueError(f"{filename} 顶层必须为对象")
                merge_config(config, defaults)
            if option in arguments:
                path = PROJECT_ROOT / Path(arguments[option]).expanduser()
                with path.open(encoding="utf-8") as stream:
                    custom = yaml.safe_load(stream)
                if not isinstance(custom, dict):
                    raise ValueError(f"{path} 顶层必须为对象")
                merge_config(config, custom)
            configs[option] = config
        values = configs[option]
        for field in parameter_path:
            if not isinstance(values, dict) or field not in values:
                raise ValueError(f"{node} 的 YAML 参数结构无效")
            values = values[field]
        if not isinstance(values, dict):
            raise ValueError(f"{node} 的 YAML 参数必须为对象")
        for key, value in overrides.items():
            if key not in values:
                raise ValueError(f"parameters.{node}.{key} 未在基础 YAML 中定义")
            if value is not None:
                values[key] = parameter_value(value, values[key], f"{node}.{key}")

    for option, config in configs.items():
        text = yaml.safe_dump(config, allow_unicode=True, sort_keys=False)
        digest = hashlib.sha256(text.encode("utf-8")).hexdigest()
        directory = PROJECT_ROOT / ".cache" / "startup" / digest
        directory.mkdir(parents=True, exist_ok=True)
        destination = directory / f"{option}.yaml"
        # 原子替换避免另一个启动进程读到未写完的配置
        stream = tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=directory,
                                              delete=False)
        temporary = Path(stream.name)
        try:
            with stream:
                stream.write(text)
            os.replace(temporary, destination)
        finally:
            temporary.unlink(missing_ok=True)
        arguments[option] = str(destination)


def load_arguments(config_path, overrides):
    """读取配置，按字段应用命令行覆盖，返回 key:=value 参数列表"""
    config_path = config_path.expanduser().resolve()
    with config_path.open(encoding="utf-8") as stream:
        config = json.load(stream)
    if not isinstance(config, dict):
        raise ValueError("JSON 顶层必须为对象")
    parameters = config.pop("parameters", {})
    if not isinstance(parameters, dict):
        raise ValueError("parameters 必须为节点参数对象")

    arguments = {}
    for key, value in config.items():
        if key not in OPTIONS:
            raise ValueError(f"未知配置项 {key}")
        if value is None:
            continue
        if key in BOOLEAN_OPTIONS:
            if type(value) is not bool:
                raise ValueError(f"{key} 必须为 true、false 或 null")
            value = str(value).lower()
        elif not isinstance(value, str) or not value.strip():
            raise ValueError(f"{key} 必须为非空字符串或 null")
        if key in PATH_OPTIONS:
            path = Path(value).expanduser()
            value = str((config_path.parent / path).resolve())
        arguments[key] = value

    for argument in overrides:
        key, separator, value = argument.partition(":=")
        if not separator or key not in OPTIONS:
            raise ValueError(f"未知或无效参数 {argument}")
        arguments[key] = value

    for key, value in arguments.items():
        if not value or any(character in value for character in "\r\n\0"):
            raise ValueError(f"{key} 的值为空或包含无效字符")
        if key in BOOLEAN_OPTIONS and value not in {"true", "false"}:
            raise ValueError(f"{key} 必须为 true 或 false")
        if key == "mode" and value not in {"mtc", "vision", "vision_pcl"}:
            raise ValueError("mode 必须为 mtc 或 vision，vision_pcl 为兼容别名")
    # 比赛模式使用包内默认配置，启动开关由 run.sh 统一设置
    if arguments.get("debug", "true") == "false":
        arguments = {key: value for key, value in arguments.items() if key in {"debug", "use_rviz"}}
    else:
        apply_parameter_overrides(arguments, parameters)
    return [f"{key}:={value}" for key, value in arguments.items()]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("config", type=Path)
    parser.add_argument("overrides", nargs="*")
    args = parser.parse_args()
    try:
        arguments = load_arguments(args.config, args.overrides)
    except (OSError, ValueError, yaml.YAMLError) as error:
        print(f"启动配置错误 {args.config} — {error}", file=sys.stderr)
        return 2
    for argument in arguments:
        print(argument)
    return 0


if __name__ == "__main__":
    sys.exit(main())
