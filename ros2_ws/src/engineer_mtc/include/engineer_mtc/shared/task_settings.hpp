#pragma once
#include <Eigen/Geometry>
#include <string>
#include <vector>

namespace engineer_mtc
{
/// 取矿与存车共用的规划参数
struct TaskSettings
{
  std::string group;      ///< 机械臂规划组。
  std::string tip;        ///< 规划末端连杆。
  std::string base_frame; ///< 基座参考坐标系。
  std::string pipeline;   ///< MoveIt 规划管线。
  std::string planner_id; ///< 管线内的规划器 ID。
  double planning_time;   ///< 运动阶段的规划预算，单位为秒。
  double velocity_scale;
  double acceleration_scale;
  double cartesian_step; ///< 笛卡尔直线路径的插值步长。
  double distance;       ///< 预抓取 TCP 沿接近方向反向退开的距离。
  bool include_approach;
  unsigned int max_ik_solutions;      ///< 存车 IK 候选上限及 OMPL 尝试次数。
  unsigned int pick_max_ik_solutions; ///< 取矿预抓取 IK 候选上限。
  Eigen::Vector3d approach_axis;      ///< TCP 坐标系下的接近方向。
  Eigen::Isometry3d tip_to_tcp;       ///< 规划末端到 TCP 的变换。
  std::string gripper_group = "gripper";
  std::string object_id = "energy";
  std::vector<std::string> touch_links = {"link_6"};
  Eigen::Vector3d extraction_axis{.000000116949, .002550757964758, -.999996746811604}; ///< 矿坐标系下的拔离方向。
  Eigen::Vector3d storage_axis{0.0, 0.0, -1.0}; ///< 基座坐标系下的入槽方向。
  std::vector<std::string> storage_contact_objects; ///< 允许存车接触的场景物体 ID。
  double extraction_distance = 0.10; ///< 持矿拔离距离。
  double storage_distance = 0.10;    ///< 直线入槽距离。
  double retreat_distance = 0.07;    ///< 松爪后的退爪距离。
};

} // namespace engineer_mtc
