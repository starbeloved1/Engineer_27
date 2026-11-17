#include "engineer_mtc/mtc_node.hpp"
#include <cmath>
#include <functional>
#include <map>
#include <moveit/task_constructor/storage.h>

namespace engineer_mtc
{
  
void MtcNode::prepareExecution()
{
  std::array<bool, 6> found{};
  std::function<void(const moveit::task_constructor::SolutionBase &)> visit;
  visit = [&](const auto &solution)
  {
    const auto *creator = solution.creator();
    const std::string name = creator ? creator->name() : "";
    if (name.rfind("stage_", 0) == 0 && name.size() > 6 && name[6] >= '1' && name[6] <= '6')
    {
      const size_t i = name[6] - '1';
      if (found[i])
        throw std::runtime_error("业务阶段解重复");
      stage_solutions_[i] = moveit_task_constructor_msgs::msg::Solution();
      solution.toMsg(stage_solutions_[i], &task_->introspection());
      if (stage_solutions_[i].sub_trajectory.empty())
        throw std::runtime_error("业务阶段解为空");
      found[i] = true;
      return;
    }
    if (const auto *sequence =
            dynamic_cast<const moveit::task_constructor::SolutionSequence *>(&solution))
      for (const auto *sub : sequence->solutions())
        visit(*sub);
    else if (const auto *wrapped =
                 dynamic_cast<const moveit::task_constructor::WrappedSolution *>(&solution))
      visit(*wrapped->wrapped());
  };
  visit(*task_->solutions().front());
  for (bool f : found)
    if (!f)
      throw std::runtime_error("完整解缺少业务阶段");
}

void MtcNode::checkStart(const moveit_task_constructor_msgs::msg::Solution &solution)
{
  sensor_msgs::msg::JointState::ConstSharedPtr current;
  {
    std::lock_guard<std::mutex> l(input_mutex_);
    current = joints_;
  }
  if (!current || (now() - rclcpp::Time(current->header.stamp)).seconds() > max_joint_age_)
    throw std::runtime_error("执行前关节反馈已过期");
  std::map<std::string, double> actual;
  for (size_t i = 0; i < current->name.size() && i < current->position.size(); ++i)
    actual[current->name[i]] = current->position[i];
  const auto &start = solution.start_scene.robot_state.joint_state;
  if (start.name.empty() || start.name.size() != start.position.size())
    throw std::runtime_error("阶段缺少完整起点状态");
  for (size_t i = 0; i < start.name.size(); ++i)
  {
    auto it = actual.find(start.name[i]);
    if (it == actual.end() || !std::isfinite(it->second))
      throw std::runtime_error("缺少关节反馈: " + start.name[i]);
    double delta = it->second - start.position[i];
    if (start.name[i] != "gripper_state_joint")
      delta = std::atan2(std::sin(delta), std::cos(delta));
    const double tolerance = start.name[i] == "gripper_state_joint" ? 1e-4 : .01;
    if (std::abs(delta) > tolerance)
      throw std::runtime_error("实际起点偏离规划: " + start.name[i]);
  }
}

bool MtcNode::gripperAt(bool closed)
{
  std::lock_guard<std::mutex> l(input_mutex_);
  if (!joints_ || (now() - rclcpp::Time(joints_->header.stamp)).seconds() > max_joint_age_)
    return false;
  for (size_t i = 0; i < joints_->name.size() && i < joints_->position.size(); ++i)
    if (joints_->name[i] == "gripper_state_joint")
      return std::abs(joints_->position[i] - (closed ? 0. : .01)) < 1e-4;
  return false;
}

void MtcNode::startExecution(bool next, Trigger::Response &response)
{
  std::lock_guard<std::mutex> command(command_mutex_);
  if (busy_.exchange(true))
  {
    response.message = "任务忙，不排队";
    return;
  }
  try
  {
    uint8_t phase;
    {
      std::lock_guard<std::mutex> l(state_mutex_);
      phase = status_.phase;
      cancel_requested_ = false;
    }
    if (!plan_valid_ || (next ? phase != Status::WAITING_NEXT : phase != Status::READY))
      throw std::runtime_error(next ? "当前没有等待下一步的任务" : "没有可执行的完整新规划");
    if (next && execution_mode_ != "step")
      throw std::runtime_error("next_stage 仅用于 step 模式");
    if (!next)
    {
      execution_mode_ = get_parameter("execution_mode").as_string();
      if (execution_mode_ != "auto" && execution_mode_ != "step")
        throw std::runtime_error("execution_mode 必须是 auto/step");
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        status_.execution_mode = execution_mode_;
      }
    }
    checkStart(stage_solutions_.at(next_stage_));
    if (!execute_client_->wait_for_action_server(std::chrono::seconds(2)))
      throw std::runtime_error("ExecuteTaskSolution 未就绪，请检查 move_group capability");
    if (worker_.joinable())
      worker_.join();
    worker_ = std::thread(&MtcNode::runExecution, this);
    response.success = true;
    response.message = "执行请求已接受，请订阅 pick_status 查看结果";
  }
  catch (const std::exception &e)
  {
    busy_ = false;
    response.message = e.what();
  }
}

void MtcNode::runExecution()
{
  struct Release
  {
    std::atomic<bool> &flag;
    ~Release()
    {
      flag = false;
    }
  } release{busy_};
  try
  {
    while (next_stage_ < 6)
    {
      if (cancel_requested_ || !rclcpp::ok())
        throw std::runtime_error("执行已取消");
      const auto index = next_stage_;
      checkStart(stage_solutions_[index]);
      setStatus(Status::EXECUTING, index + 1, "执行本阶段，等待控制器及场景反馈");
      Execute::Goal goal;
      goal.solution = stage_solutions_[index];
      auto options = rclcpp_action::Client<Execute>::SendGoalOptions();
      std::weak_ptr<MtcNode> weak = std::static_pointer_cast<MtcNode>(shared_from_this());
      options.goal_response_callback = [weak](GoalHandle::SharedPtr handle)
      {
        if (auto node = weak.lock(); node && handle && node->cancel_requested_)
          node->execute_client_->async_cancel_goal(handle);
      };
      auto accepted = execute_client_->async_send_goal(goal, options);
      const auto accept_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
      while (accepted.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready)
        if (!rclcpp::ok() || std::chrono::steady_clock::now() > accept_deadline)
        {
          cancel_requested_ = true;
          throw std::runtime_error("执行服务未确认接收");
        }
      auto handle = accepted.get();
      if (!handle)
        throw std::runtime_error("执行请求被 MoveIt 拒绝");
      {
        std::lock_guard<std::mutex> l(goal_mutex_);
        active_goal_ = handle;
      }
      if (cancel_requested_)
        execute_client_->async_cancel_goal(handle);
      double seconds = 0;
      for (const auto &segment : goal.solution.sub_trajectory)
        if (!segment.trajectory.joint_trajectory.points.empty())
        {
          const auto &t = segment.trajectory.joint_trajectory.points.back().time_from_start;
          seconds += t.sec + t.nanosec * 1e-9;
        }
      auto result = execute_client_->async_get_result(handle);
      const auto deadline =
          std::chrono::steady_clock::now() + std::chrono::duration<double>(seconds * 1.5 + 10.);
      bool cancel_sent = cancel_requested_;
      while (result.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready)
      {
        if (!rclcpp::ok())
          throw std::runtime_error("ROS 正在关闭");
        if (cancel_requested_ && !cancel_sent)
        {
          execute_client_->async_cancel_goal(handle);
          cancel_sent = true;
        }
        if (std::chrono::steady_clock::now() > deadline)
        {
          execute_client_->async_cancel_goal(handle);
          throw std::runtime_error("控制器执行超时，已请求取消，后续阶段停止");
        }
      }
      const auto completed = result.get();
      {
        std::lock_guard<std::mutex> l(goal_mutex_);
        active_goal_.reset();
      }
      if (cancel_requested_)
        throw std::runtime_error("执行已取消");
      if (completed.code != rclcpp_action::ResultCode::SUCCEEDED || !completed.result ||
          completed.result->error_code.val != moveit_msgs::msg::MoveItErrorCodes::SUCCESS)
        throw std::runtime_error(
            "阶段执行失败，MoveIt code=" +
            std::to_string(completed.result ? completed.result->error_code.val : 0));
      // 不靠已发送命令推测抓取状态；查询实际规划场景和模拟关节反馈。
      const bool expected_attached = index >= 1 && index <= 3;
      const bool expected_closed = index == 1 || index == 2 || index == 3 || index == 5;
      bool confirmed = false;
      for (int retry = 0; retry < 20 && rclcpp::ok() && !cancel_requested_; ++retry)
      {
        if (energy_->attached() == expected_attached && gripperAt(expected_closed))
        {
          confirmed = true;
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      if (!confirmed)
        throw std::runtime_error("阶段反馈不符合预期：夹爪或矿附着状态未确认");
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        status_.object_attached = expected_attached;
      }
      ++next_stage_;
      if (next_stage_ == 6)
      {
        plan_valid_ = false;
        setStatus(Status::SUCCEEDED, 6, "完整 pick 完成：矿已存放，机械臂已回位");
        return;
      }
      if (execution_mode_ == "step")
      {
        setStatus(Status::WAITING_NEXT, index + 1, "本阶段完成，等待 next_stage");
        return;
      }
    }
  }
  catch (const std::exception &e)
  {
    plan_valid_ = false;
    if (rclcpp::ok())
    {
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        status_.object_attached = energy_->attached();
      }
      setStatus(cancel_requested_ ? Status::CANCELED : Status::FAILED,
                static_cast<uint8_t>(std::min(next_stage_ + 1, size_t(6))), e.what());
    }
  }
}

void MtcNode::cancel(Trigger::Response &response)
{
  std::lock_guard<std::mutex> command(command_mutex_);
  cancel_requested_ = true;
  {
    std::lock_guard<std::mutex> l(goal_mutex_);
    if (active_goal_)
      execute_client_->async_cancel_goal(active_goal_);
  }
  uint8_t phase, stage;
  {
    std::lock_guard<std::mutex> l(state_mutex_);
    phase = status_.phase;
    stage = status_.stage;
  }
  // READY/WAITING_NEXT 已经没有正在求解或执行的工作；即使工作线程
  // 尚在退出，也要立即作废待执行解。活动工作则由其确认停止后发布终态。
  if (!busy_ || phase == Status::READY || phase == Status::WAITING_NEXT)
  {
    plan_valid_ = false;
    setStatus(Status::CANCELED, stage, "任务已取消；保留实际持物状态，不自动松爪或回位");
  }
  response.success = true;
  response.message = "取消请求已接收；运动停止结果见 pick_status";
}
} // namespace engineer_mtc
