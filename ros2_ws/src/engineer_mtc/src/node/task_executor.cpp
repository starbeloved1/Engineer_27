#include "engineer_mtc/node/mtc_node.hpp"
#include "engineer_mtc/shared/motion_stages.hpp"
#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <moveit/robot_model/robot_model.h>
#include <moveit/task_constructor/storage.h>

namespace engineer_mtc
{
void MtcNode::checkHardware()
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  if (!hardware_ || !hardware_->feedback_valid || hardware_->faulted ||
      std::chrono::duration<double>(std::chrono::steady_clock::now() - hardware_received_).count() > hardware_timeout_ ||
      std::abs((now() - rclcpp::Time(hardware_->header.stamp)).seconds()) > hardware_timeout_)
    throw std::runtime_error("硬件反馈失效或超时");
  if (hardware_->mode != 2 || !hardware_->tx_enabled || (hardware_->gripper_state & 6))
    throw std::runtime_error("当前未取得自动取矿控制权，或夹爪存在故障／堵转");
}

bool MtcNode::executionNeedsHardware()
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  return status_.phase == Status::EXECUTING || status_.phase == Status::WAITING_NEXT;
}

void MtcNode::updateHardware(serial_driver::msg::HardwareStatus::ConstSharedPtr message)
{
  bool changed;
  {
    std::lock_guard<std::mutex> lock(input_mutex_);
    changed = hardware_ && hardware_->control_epoch != message->control_epoch;
    hardware_ = message;
    hardware_received_ = std::chrono::steady_clock::now();
  }
  // busy_ 也覆盖纯规划，plan_valid_ 仅表示有轨迹；两者均不要求发送使能。
  // 执行前仍由 checkStart 严格检查，执行及分步等待期间持续监测。
  if (executionNeedsHardware())
  {
    std::string reason = changed ? "硬件控制权发生变化" : "";
    try { checkHardware(); } catch (const std::exception &e) { reason = e.what(); }
    if (!reason.empty() && !cancel_requested_)
    {
      RCLCPP_WARN(get_logger(), "硬件监测取消任务: %s", reason.c_str());
      Trigger::Response response;
      cancel(response);
    }
  }
}

void MtcNode::prepareExecution(moveit::task_constructor::Task &task,
                               const moveit::task_constructor::SolutionBase &selected,
                               size_t first_stage, size_t end_stage)
{
  (void)task;  // 解可直接序列化，避免访问 introspection 时重新创建可视化线程。
  std::array<bool, 6> found{};
  std::function<void(const moveit::task_constructor::SolutionBase &)> visit;
  visit = [&](const auto &solution)
  {
    const auto *creator = solution.creator();
    const std::string name = creator ? creator->name() : "";
    if (name.rfind("stage_", 0) == 0 && name.size() > 6 && name[6] >= '1' && name[6] <= '6')
    {
      const size_t i = name[6] - '1';
      if (i < first_stage || i >= end_stage || found[i])
        throw std::runtime_error("业务阶段解超出当前任务范围或重复");
      stage_solutions_[i] = moveit_task_constructor_msgs::msg::Solution();
      solution.toMsg(stage_solutions_[i]);
      if (stage_solutions_[i].sub_trajectory.empty())
        throw std::runtime_error("业务阶段解为空");
      // MTC can serialize backward and connecting stages as full scenes. Only
      // forward scene stages here change objects, and they already emit diffs.
      // A full scene would discard obstacles added since the task was planned.
      for (auto &segment : stage_solutions_[i].sub_trajectory)
      {
        if (!segment.scene_diff.is_diff)
        {
          moveit_msgs::msg::PlanningScene diff;
          diff.is_diff = true;
          diff.robot_state.is_diff = true;
          diff.allowed_collision_matrix = std::move(segment.scene_diff.allowed_collision_matrix);
          segment.scene_diff = std::move(diff);
        }
        // 执行器临时应用许可。入槽阶段保留解附着和退爪的 ACM 更新。
        if (i != 4)
          segment.scene_diff.allowed_collision_matrix = moveit_msgs::msg::AllowedCollisionMatrix{};
      }
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
  visit(selected);
  for (size_t i = first_stage; i < end_stage; ++i)
    if (!found[i])
      throw std::runtime_error("轨迹缺少阶段 " + std::to_string(i + 1));
}

void MtcNode::checkTrajectoryLimits(const moveit_task_constructor_msgs::msg::Solution &solution)
{
  // 检查每个轨迹点，continuous 关节也须符合串口协议的 ±180° 范围。
  constexpr double pi = 3.14159265358979323846;
  for (const auto &segment : solution.sub_trajectory)
  {
    const auto &trajectory = segment.trajectory.joint_trajectory;
    for (const auto &point : trajectory.points)
    {
      if (point.positions.size() != trajectory.joint_names.size())
        throw std::runtime_error("关节轨迹位置数量不匹配");
      for (size_t i = 0; i < point.positions.size(); ++i)
      {
        const double q = point.positions[i];
        const bool gripper = trajectory.joint_names[i] == "gripper_state_joint";
        if (!std::isfinite(q) || (gripper ? (q < -1e-10 || q > .01 + 1e-10) : std::abs(q) > pi + 1e-10))
          throw std::runtime_error("轨迹超出串口位置范围: " + trajectory.joint_names[i]);
      }
    }
  }
}

void MtcNode::checkStart(const moveit_task_constructor_msgs::msg::Solution &solution)
{
  checkHardware();
  checkTrajectoryLimits(solution);
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
    const bool gripper = start.name[i] == "gripper_state_joint";
    if (!gripper)
      delta = std::atan2(std::sin(delta), std::cos(delta));
    // 串口联调期间，仅对机械臂关节检查起点位置偏差。
    // const double tolerance = gripper ? 1e-4 : 0.1;
    // if (std::abs(delta) > tolerance)
    if (!gripper && std::abs(delta) > 0.1)
      throw std::runtime_error("实际起点偏离规划: " + start.name[i]);
  }
}

bool MtcNode::gripperAt(bool closed)
{
  std::lock_guard<std::mutex> l(input_mutex_);
  if (!hardware_ || !hardware_->feedback_valid || hardware_->faulted ||
      (hardware_->gripper_state & 7) != 1 ||
      std::abs((now() - rclcpp::Time(hardware_->header.stamp)).seconds()) > hardware_timeout_) return false;
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
    if (execution_unconfirmed_)
      throw std::runtime_error("旧轨迹是否停止或碰撞许可是否恢复尚未确认");
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
  bool contact_in_progress = false;
  bool place_contacts_active = false;
  bool stopping_timed_out = false;
  const bool pick = end_stage_ == 3;
  std::optional<moveit_msgs::msg::AllowedCollisionMatrix> pick_collision_baseline;
  const CollisionMatrix baseline(baseline_acm_);
  const auto &model = *task_->getRobotModel();
  const ContactPairs arm_base(baseline, model.getRootLinkName(),
                              model.getJointModelGroup(settings_.group)->getLinkModelNames());
  const auto carried = pick ? ContactPairs{} :
      carriedPlaceContacts(baseline, settings_, model.getLinkModelNames());
  const ContactPairs tool_slot(baseline, settings_.tip, settings_.storage_contact_objects);
  const auto set_place_contacts = [&](size_t index, bool allow)
  {
    if (index < 5)
      energy_->setContacts({arm_base, carried, tool_slot}, allow);
    else
      energy_->setContacts({arm_base}, allow);
  };
  try
  {
    while (next_stage_ < end_stage_)
    {
      if (cancel_requested_ || !rclcpp::ok())
        throw std::runtime_error("执行已取消");
      const auto index = next_stage_;
      checkStart(stage_solutions_[index]);
      setStatus(Status::EXECUTING, index + 1, "执行本阶段，等待控制器及场景反馈");
      if (pick)
      {
        pick_collision_baseline = energy_->collisionMatrix();
        energy_->allowPickCollisions(task_->getRobotModel()->getLinkModelNames(), *pick_collision_baseline);
      }
      else
      {
        place_contacts_active = true;
        set_place_contacts(index, true);
      }
      if (cancel_requested_)
        throw std::runtime_error("执行已取消");
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
          execution_unconfirmed_ = true;
          throw std::runtime_error("执行服务未确认接收，需检查实际动作状态");
        }
      auto handle = accepted.get();
      if (!handle)
        throw std::runtime_error("执行请求被 MoveIt 拒绝");
      {
        std::lock_guard<std::mutex> l(goal_mutex_);
        active_goal_ = handle;
      }
      contact_in_progress = index == 1 || index == 4;
      if (index == 4)
        extracted_ = false;
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
      const auto stop_deadline = deadline + std::chrono::seconds(5);
      while (result.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready)
      {
        if (!rclcpp::ok())
          throw std::runtime_error("ROS 正在关闭");
        if (cancel_requested_ && !cancel_sent)
        {
          execute_client_->async_cancel_goal(handle);
          cancel_sent = true;
        }
        const auto now_steady = std::chrono::steady_clock::now();
        if (now_steady > deadline && !stopping_timed_out)
        {
          stopping_timed_out = true;
          if (!cancel_sent)
            execute_client_->async_cancel_goal(handle);
          cancel_sent = true;
        }
        if (now_steady > stop_deadline)
        {
          execution_unconfirmed_ = true;
          throw std::runtime_error("控制器执行超时，尚未确认停止");
        }
      }
      const auto completed = result.get();
      {
        std::lock_guard<std::mutex> l(goal_mutex_);
        active_goal_.reset();
      }
      if (pick_collision_baseline)
      {
        energy_->setCollisionMatrix(*pick_collision_baseline);
        pick_collision_baseline.reset();
      }
      if (place_contacts_active)
      {
        set_place_contacts(index, false);
        place_contacts_active = false;
      }
      if (stopping_timed_out)
        throw std::runtime_error("控制器执行超时，已收到终态");
      if (cancel_requested_)
        throw std::runtime_error("执行已取消");
      if (completed.code != rclcpp_action::ResultCode::SUCCEEDED || !completed.result ||
          completed.result->error_code.val != moveit_msgs::msg::MoveItErrorCodes::SUCCESS)
        throw std::runtime_error(
            "阶段执行失败，MoveIt code=" +
            std::to_string(completed.result ? completed.result->error_code.val : 0));
      // 确认规划场景更新；串口联调期间暂停夹爪到位判定。
      const bool expected_attached = index >= 1 && index <= 3;
      // const bool expected_closed = index == 1 || index == 2 || index == 3 || index == 5;
      bool confirmed = false;
      for (int retry = 0; retry < 20 && rclcpp::ok() && !cancel_requested_; ++retry)
      {
        if (energy_->attached() == expected_attached &&
            // gripperAt(expected_closed) &&
            (index != 4 || energy_->stored(storedObjectId(settings_.object_id))))
        {
          confirmed = true;
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      if (!confirmed)
        throw std::runtime_error("阶段反馈不符合预期：矿场景状态未确认");
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        status_.object_attached = expected_attached;
      }
      contact_in_progress = false;
      ++next_stage_;
      if (next_stage_ == end_stage_)
      {
        plan_valid_ = false;
        if (end_stage_ == 3)
        {
          extracted_ = true;
          setStatus(Status::SUCCEEDED, 3, "矿已拔离并保持附着，可调用 plan_place");
        }
        else
        {
          {
            std::lock_guard<std::mutex> scene_lock(scene_mutex_);
            preview_visible_ = false;
            owned_ = false;
          }
          setStatus(Status::SUCCEEDED, 6, "矿已存放，机械臂已回位");
        }
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
      std::string detail = e.what();
      if (pick_collision_baseline)
      {
        try
        {
          energy_->setCollisionMatrix(*pick_collision_baseline);
        }
        catch (const std::exception &error)
        {
          execution_unconfirmed_ = true;
          detail += std::string("；恢复取矿前的碰撞矩阵失败：") + error.what();
        }
      }
      if (place_contacts_active && !execution_unconfirmed_)
      {
        try
        {
          set_place_contacts(next_stage_, false);
        }
        catch (const std::exception &error)
        {
          execution_unconfirmed_ = true;
          detail += "；";
          detail += error.what();
        }
      }
      if (contact_in_progress)
        contact_state_needs_review_ = true;
      const bool attached = energy_->attached();
      owned_ = attached || contact_state_needs_review_ || execution_unconfirmed_;
      {
        std::lock_guard<std::mutex> l(state_mutex_);
        status_.object_attached = attached;
      }
      if (end_stage_ == 6 && next_stage_ == 5)
        detail = "矿已释放并完成退爪，回位未完成：" + detail;
      setStatus(cancel_requested_ ? Status::CANCELED : Status::FAILED,
                static_cast<uint8_t>(std::min(next_stage_ + 1, end_stage_)), detail);
    }
  }
}

void MtcNode::cancel(Trigger::Response &response)
{
  std::lock_guard<std::mutex> command(command_mutex_);
  cancel_requested_ = true;
  {
    std::lock_guard<std::mutex> lock(planning_mutex_);
    if (active_planning_task_) active_planning_task_->preempt();
  }
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
