#pragma once
#include "engineer_mtc/shared/grasp_geometry.hpp"
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <moveit_msgs/srv/get_planning_scene.hpp>
#include <rclcpp/rclcpp.hpp>
#include <functional>
#include <initializer_list>
#include <string>
#include <mutex>
#include <vector>

namespace engineer_mtc
{
class ContactPairs;

/// 管理矿和碰撞场景
class EnergyScene
{
public:
  /// 从 path 读取矿 URDF，使用 id 标识物体，frame 指定参考坐标系。
  EnergyScene(const std::string &path, const std::string &id, const std::string &frame);
  /// 以基座下矿位姿添加或更新碰撞体，返回是否成功；已附着时返回 false。
  bool put(const Eigen::Isometry3d &base_energy);
  /// 移除场景中的目标矿。
  void remove();
  /// 返回目标矿是否已附着到机器人。
  bool attached();
  /// 返回场景中是否存在给定 ID 的已存矿。
  bool stored(const std::string &id);
  /// 查询矿相对 tip 的持矿变换，要求矿附着在该末端。
  Eigen::Isometry3d attachedPose(const std::string &tip);
  /// 检查存矿位未占用，且配置的槽内接触物体均存在。
  void checkStorage(const std::string &stored_id, const std::vector<std::string> &contact_objects);
  /// 读取当前规划场景的碰撞矩阵。
  moveit_msgs::msg::AllowedCollisionMatrix collisionMatrix();
  /// 在 baseline 基础上放宽给定连杆与场景物体的取矿碰撞许可。
  void allowPickCollisions(const std::vector<std::string> &links,
                           const moveit_msgs::msg::AllowedCollisionMatrix &baseline);
  /// 将给定碰撞矩阵应用到规划场景。
  void setCollisionMatrix(const moveit_msgs::msg::AllowedCollisionMatrix &matrix);
  /// allow 为 true 时放行给定接触对，为 false 时恢复各对原许可。
  void setContacts(std::initializer_list<std::reference_wrapper<const ContactPairs>> contacts, bool allow);

private:
  /// 通过场景服务读取碰撞矩阵，调用方持有 mutex_。
  moveit_msgs::msg::AllowedCollisionMatrix readCollisionMatrix();
  /// 通过场景接口提交碰撞矩阵，调用方持有 mutex_。
  void applyCollisionMatrix(const moveit_msgs::msg::AllowedCollisionMatrix &matrix);
  rclcpp::Node::SharedPtr service_node_;
  rclcpp::Client<moveit_msgs::srv::GetPlanningScene>::SharedPtr scene_client_;
  moveit_msgs::msg::CollisionObject model_;
  moveit::planning_interface::PlanningSceneInterface scene_;
  std::mutex mutex_; // PlanningSceneInterface 内部服务节点不可被多个线程同时 spin。
};
} // namespace engineer_mtc
