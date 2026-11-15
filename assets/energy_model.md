# 能量单元视觉模型

当前唯一运行模型为 `assets/energy.ply`，复制自参考仓库 `engineer_2026` 的 `jetson` 分支。
SHA-256：`d679b7ca1bb00719eb290fc27475b6959ac938a8acd3fc8ef75425dec5fcd737`。

PLY 含 1124 个原始顶点，按毫米解释，乘 `0.001` 转成米，保留 CAD 原点与坐标轴。
模型发布流程为读取顶点 → 随机限点最多 5000 → 5 mm 体素；ICP 接收后再做 6 mm 体素。
不额外生成表面采样点。原始 Z 范围约 `[-70.665, 79.443] mm`。

`/vision/model_cloud` 使用独立 `energy_model` 坐标系，没有相机固定 TF。
配准通过后，`/vision/icp/raw_object_poses` 和 `raw_aligned_cloud` 给出原始 ICP 完整变换，
`object_poses` 和 `aligned_cloud` 给出 Z 轴处理及启用的六矿约束之后的结果。
这些输出使用配置中的 `target_frame`，当前为 `camera_link`。

位姿描述 PLY 模型原点，不是观测点质心或夹爪 TCP。对称、遮挡、初始朝向及局部极值可能导致错误配准，
拟合分数不能直接代表实物定位精度。详见 [视觉包说明](../ros2_ws/src/vision_nodes/README.md)。

`assets/energy.stl` 保留为 CAD 参考资料，不参与当前构建和运行。
其 SHA-256 为 `6f5b9df23e3880a9df0ac9860144627444e5ca331556d0175201f1330be0f1d9`，
Z 范围约 `[-0.089, 149.989] mm`，与 PLY 原点不同，不能直接互换两者的位姿。
