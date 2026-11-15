# PCL 的 Eigen 查找模块可能只设置变量；导出库还需要显式创建 Eigen3::Eigen。
include(CMakeFindDependencyMacro)
find_dependency(Eigen3 CONFIG)
