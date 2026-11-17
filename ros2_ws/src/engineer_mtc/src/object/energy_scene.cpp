#include "engineer_mtc/energy_scene.hpp"
#include <shape_msgs/msg/solid_primitive.hpp>
#include <urdf/model.h>

// 矿
namespace engineer_mtc
{

// 读取urdf
EnergyScene::EnergyScene(const std::string &path, const std::string &id, const std::string &frame)
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
  if (attached())
    return false;
  auto object = model_;
  object.pose = transformPose(pose);
  return scene_.applyCollisionObject(object);
}

// 删除矿
void EnergyScene::remove()
{
  scene_.removeCollisionObjects({model_.id});
}

// 查询矿和机械臂的附着情况
bool EnergyScene::attached()
{
  return !scene_.getAttachedObjects({model_.id}).empty();
}

} // namespace engineer_mtc
