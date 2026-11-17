#include "engineer_mtc/pick_stages.hpp"
namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage5(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_5: store and retreat");
  
  // 建立直线段
  {
  stage->insert(relativeMove(c, "lower to storage",
                            -c.settings.storage_energy.linear() * c.settings.extraction_axis,
                            c.settings.storage_distance, true));
  }

  // ik求解并向下放入
  {
  auto pose = std::make_unique<mtc::stages::GeneratePose>("storage pose");
  pose->setMonitoredStage(c.extracted_state);
  geometry_msgs::msg::PoseStamped goal;
  goal.header.frame_id = c.settings.base_frame;
  goal.pose = transformPose(c.storage_tip);
  pose->setPose(goal);
  auto ik = std::make_unique<mtc::stages::ComputeIK>("storage IK", std::move(pose));
  ik->setGroup(c.settings.group);
  ik->setIKFrame(c.settings.tip);
  ik->setMaxIKSolutions(c.settings.max_ik_solutions);
  ik->setMinSolutionDistance(.1);
  ik->setTimeout(std::min(1., c.settings.planning_time * .2));
  ik->properties().configureInitFrom(mtc::Stage::INTERFACE, {"target_pose"});
  stage->insert(std::move(ik));
  }

  // 打开夹爪，解除附着状态并退出存矿位置
  {
  stage->insert(gripperMove(c, false));
  auto detach = std::make_unique<mtc::stages::ModifyPlanningScene>("detach energy");
  detach->detachObject(c.settings.object_id, c.settings.tip);
  stage->insert(std::move(detach));
  stage->insert(relativeMove(c, "retreat empty gripper",
                             -c.storage_tip.linear() * c.settings.tip_to_tcp.linear() *
                                 c.settings.approach_axis,
                             c.settings.retreat_distance));
  }

  // 对齐 26：存矿后不恢复接触碰撞限制，下一阶段清理场景后回位。

  return stage;
}
} // namespace engineer_mtc
