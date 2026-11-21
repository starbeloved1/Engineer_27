#include "engineer_mtc/energy_scene.hpp"
#include "motion_stages.hpp"
#include <moveit_msgs/msg/planning_scene_components.hpp>
#include <shape_msgs/msg/solid_primitive.hpp>
#include <urdf/model.h>

// 矿
namespace engineer_mtc
{

// 读取urdf
EnergyScene::EnergyScene(const std::string &path, const std::string &id, const std::string &frame)
  : service_node_(std::make_shared<rclcpp::Node>(
        "engineer_mtc_scene_client", rclcpp::NodeOptions()
          .use_global_arguments(false).start_parameter_services(false).start_parameter_event_publisher(false))),
    scene_client_(service_node_->create_client<moveit_msgs::srv::GetPlanningScene>("/get_planning_scene"))
{
  urdf::Model model;
  if (!model.initFile(path))
    throw std::runtime_error("无法读取矿 URDF: " + path);
  model_.id = id;
  model_.header.frame_id = frame;
  model_.operation = moveit_msgs::msg::CollisionObject::ADD;
  model_.pose.orientation.w = 1;
  for (const auto &collision : model.getRoot()->collision_array)
  {
    shape_msgs::msg::SolidPrimitive primitive;
    if (const auto box = std::dynamic_pointer_cast<urdf::Box>(collision->geometry))
    {
      primitive.type = primitive.BOX;
      primitive.dimensions = {box->dim.x, box->dim.y, box->dim.z};
    }
    else if (const auto cylinder = std::dynamic_pointer_cast<urdf::Cylinder>(collision->geometry))
    {
      primitive.type = primitive.CYLINDER;
      primitive.dimensions = {cylinder->length, cylinder->radius};
    }
    else
      throw std::runtime_error("矿碰撞模型只支持 box/cylinder");
    geometry_msgs::msg::Pose pose;
    const auto &o = collision->origin;
    pose.position.x = o.position.x;
    pose.position.y = o.position.y;
    pose.position.z = o.position.z;
    pose.orientation.x = o.rotation.x;
    pose.orientation.y = o.rotation.y;
    pose.orientation.z = o.rotation.z;
    pose.orientation.w = o.rotation.w;
    model_.primitives.push_back(primitive);
    model_.primitive_poses.push_back(pose);
  }
  if (model_.primitives.size() != 3)
    throw std::runtime_error("矿应有三个碰撞几何体");
}

// 在planningscene的指定位置加入矿
bool EnergyScene::put(const Eigen::Isometry3d &pose)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (!scene_.getAttachedObjects({model_.id}).empty())
    return false;
  auto object = model_;
  object.pose = transformPose(pose);
  return scene_.applyCollisionObject(object);
}

// 删除矿
void EnergyScene::remove()
{
  std::lock_guard<std::mutex> lock(mutex_);
  scene_.removeCollisionObjects({model_.id});
}

// 查询矿和机械臂的附着情况
bool EnergyScene::attached()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return !scene_.getAttachedObjects({model_.id}).empty();
}

bool EnergyScene::stored(const std::string &id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  return scene_.getObjects({id}).count(id) != 0;
}

Eigen::Isometry3d EnergyScene::attachedPose(const std::string &tip)
{
  std::lock_guard<std::mutex> lock(mutex_);
  const auto objects = scene_.getAttachedObjects({model_.id});
  const auto found = objects.find(model_.id);
  if (found == objects.end() || found->second.link_name != tip ||
      found->second.object.header.frame_id != tip)
    throw std::runtime_error("规划场景没有末端坐标系下的持矿位姿");
  return poseTransform(found->second.object.pose);
}

void EnergyScene::checkStorage(const std::string &stored_id,
                               const std::vector<std::string> &contact_objects)
{
  std::lock_guard<std::mutex> lock(mutex_);
  const auto objects = scene_.getObjects();
  if (objects.count(stored_id))
    throw std::runtime_error("存矿位已占用：" + stored_id);
  for (const auto &id : contact_objects)
    if (id == model_.id || id == stored_id || !objects.count(id))
      throw std::runtime_error("无效的槽内接触物体：" + id);
}

moveit_msgs::msg::AllowedCollisionMatrix EnergyScene::readCollisionMatrix()
{
  constexpr auto timeout = std::chrono::seconds(2);
  if (!scene_client_->wait_for_service(timeout))
    throw std::runtime_error("规划场景不可用，无法读取碰撞矩阵");
  auto request = std::make_shared<moveit_msgs::srv::GetPlanningScene::Request>();
  request->components.components = moveit_msgs::msg::PlanningSceneComponents::ALLOWED_COLLISION_MATRIX;
  auto future = scene_client_->async_send_request(request);
  if (rclcpp::spin_until_future_complete(service_node_, future, timeout) != rclcpp::FutureReturnCode::SUCCESS)
  {
    scene_client_->remove_pending_request(future);
    throw std::runtime_error("读取碰撞矩阵超时");
  }
  return future.get()->scene.allowed_collision_matrix;
}

moveit_msgs::msg::AllowedCollisionMatrix EnergyScene::collisionMatrix()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return readCollisionMatrix();
}

void EnergyScene::restoreContacts(const ContactPairs &gripper, const ContactPairs &slot)
{
  std::lock_guard<std::mutex> lock(mutex_);
  const auto current = readCollisionMatrix();
  CollisionMatrix matrix(current);
  gripper.restore(matrix);
  slot.restore(matrix);
  moveit_msgs::msg::PlanningScene diff;
  matrix.getMessage(diff.allowed_collision_matrix);
  if (diff.allowed_collision_matrix == current)
    return;
  diff.is_diff = true;
  diff.robot_state.is_diff = true;
  if (!scene_.applyPlanningScene(diff))
    throw std::runtime_error("无法恢复临时碰撞许可");
}

} // namespace engineer_mtc
