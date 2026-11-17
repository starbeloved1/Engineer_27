#include "engineer_mtc/pick_stages.hpp"
namespace engineer_mtc
{
std::unique_ptr<mtc::SerialContainer> createStage2(const PickContext &c)
{
  auto stage = std::make_unique<mtc::SerialContainer>("stage_2: grasp and attach");
  
  // 调用函数生成接近直线段
  {
  auto approach = relativeMove(c, "approach waist",
                               c.targets.grasp_tcp_base.linear() * c.settings.approach_axis,
                               c.settings.distance, true);
  stage->insert(std::move(approach));
  }

  // 生成抓取目标
  auto pose = std::make_unique<mtc::stages::GeneratePose>("grasp pose");
  {
  pose->setMonitoredStage(c.open_state);
  geometry_msgs::msg::PoseStamped goal;
  goal.header.frame_id = c.settings.base_frame;
  goal.pose = transformPose(c.targets.grasp_tcp_base * c.settings.tip_to_tcp.inverse());
  pose->setPose(goal);
  }

  // ik求解位姿并移动
  {
  auto ik = std::make_unique<mtc::stages::ComputeIK>("grasp IK", std::move(pose));
  ik->setGroup(c.settings.group);
  ik->setIKFrame(c.settings.tip);
  ik->setMaxIKSolutions(c.settings.max_ik_solutions);
  ik->setMinSolutionDistance(.1);
  ik->setTimeout(std::min(1., c.settings.planning_time * .2));
  ik->properties().configureInitFrom(mtc::Stage::INTERFACE, {"target_pose"});
  stage->insert(std::move(ik));
  } 

  // 闭爪附着
  {
  stage->insert(gripperMove(c, true));
  auto attach = std::make_unique<mtc::stages::ModifyPlanningScene>("attach energy");
  attach->attachObject(c.settings.object_id, c.settings.tip);
  stage->insert(std::move(attach));
  }
  
  return stage;
}
} // namespace engineer_mtc
