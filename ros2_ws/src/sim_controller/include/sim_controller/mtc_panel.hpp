#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <engineer_mtc/msg/pick_status.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rviz_common/panel.hpp>
#include <std_srvs/srv/trigger.hpp>

class QLabel;
class QPushButton;
class QTimer;

namespace sim_controller
{
/// 通过现有 MTC 服务控制规划和执行，并显示任务反馈
class MtcPanel : public rviz_common::Panel
{
  Q_OBJECT

public:
  explicit MtcPanel(QWidget *parent = nullptr);
  ~MtcPanel() override;
  void onInitialize() override;

private:
  using Trigger = std_srvs::srv::Trigger;
  using Status = engineer_mtc::msg::PickStatus;
  enum CommandIndex { Plan, PlanManual, PlanPlace, Execute, Next, Cancel, Replay, CommandCount };
  struct Command
  {
    QPushButton *button = nullptr;
    rclcpp::Client<Trigger>::SharedPtr client;
    bool pending = false;
    int64_t request_id = 0;
    std::chrono::steady_clock::time_point started;
  };

  /// 异步发送指定服务请求，结果显示在面板中
  void request(size_t index);
  /// 根据任务状态、服务连接和待返回请求更新按钮
  void updateControls();
  /// 显示节点发布的任务状态和阶段信息
  void updateStatus(const Status &status);

  std::array<Command, CommandCount> commands_;
  Status status_;
  bool status_received_ = false;
  QLabel *status_label_;
  QLabel *detail_label_;
  QLabel *response_label_;
  QTimer *timer_;
  rclcpp::Node::SharedPtr node_;
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor_;
  rclcpp::Subscription<Status>::SharedPtr status_subscription_;
};
} // namespace sim_controller
