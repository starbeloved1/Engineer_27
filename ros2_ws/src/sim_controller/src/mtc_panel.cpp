#include "sim_controller/mtc_panel.hpp"

#include <algorithm>
#include <chrono>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <pluginlib/class_list_macros.hpp>

namespace sim_controller
{
namespace
{
const std::array<const char *, 7> service_names{
    "plan", "plan_manual", "plan_place", "execute", "next_stage", "cancel", "replay"};
const std::array<const char *, 7> button_names{
    "视觉取矿规划", "手动目标规划", "持矿存车规划", "执行", "下一阶段", "取消任务", "重播规划（仅预览）"};
const std::array<const char *, 8> phase_names{
    "空闲", "规划中", "规划就绪", "执行中", "等待下一阶段", "成功", "失败", "已取消"};
} // namespace

MtcPanel::MtcPanel(QWidget *parent) : rviz_common::Panel(parent)
{
  auto *layout = new QVBoxLayout(this);
  status_label_ = new QLabel("等待 MTC 状态", this);
  detail_label_ = new QLabel(this);
  response_label_ = new QLabel(this);
  for (auto *label : {status_label_, detail_label_, response_label_})
  {
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    layout->addWidget(label);
  }
  auto *buttons = new QGridLayout;
  for (size_t i = 0; i < commands_.size(); ++i)
  {
    auto *button = new QPushButton(QString::fromUtf8(button_names[i]), this);
    button->setObjectName(service_names[i]);
    button->setEnabled(false);
    commands_[i].button = button;
    buttons->addWidget(button, static_cast<int>(i / 2), static_cast<int>(i % 2));
    connect(button, &QPushButton::clicked, this, [this, i] { request(i); });
  }
  layout->addLayout(buttons);
  layout->addStretch();
  timer_ = new QTimer(this);
  connect(timer_, &QTimer::timeout, this, [this]
  {
    if (!rclcpp::ok())
    {
      timer_->stop();
      for (auto &command : commands_) command.button->setEnabled(false);
      status_label_->setText("ROS 已关闭");
      return;
    }
    executor_->spin_some(std::chrono::milliseconds(1));
    updateControls();
  });
}

MtcPanel::~MtcPanel()
{
  timer_->stop();
  if (executor_ && node_) executor_->remove_node(node_);
}

void MtcPanel::onInitialize()
{
  node_ = std::make_shared<rclcpp::Node>(
      "engineer_mtc_rviz_panel", rclcpp::NodeOptions().use_global_arguments(false));
  executor_ = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
  executor_->add_node(node_);
  for (size_t i = 0; i < commands_.size(); ++i)
    commands_[i].client = node_->create_client<Trigger>(
        std::string("/engineer_mtc/") + service_names[i]);
  status_subscription_ = node_->create_subscription<Status>(
      "/engineer_mtc/pick_status", rclcpp::QoS(1).reliable().transient_local(),
      [this](Status::ConstSharedPtr status) { updateStatus(*status); });
  timer_->start(50);
}

void MtcPanel::request(size_t index)
{
  auto &command = commands_[index];
  if (!command.button->isEnabled() || command.pending) return;
  command.pending = true;
  command.started = std::chrono::steady_clock::now();
  response_label_->setText(QString("等待服务返回 %1").arg(service_names[index]));
  updateControls();
  try
  {
    const auto request = command.client->async_send_request(std::make_shared<Trigger::Request>(),
        [this, index](rclcpp::Client<Trigger>::SharedFuture future)
        {
          commands_[index].pending = false;
          try
          {
            const auto response = future.get();
            response_label_->setText(QString("%1 %2\n%3")
                .arg(service_names[index])
                .arg(response->success ? "已受理" : "未完成")
                .arg(QString::fromStdString(response->message)));
          }
          catch (const std::exception &error)
          {
            response_label_->setText(QString::fromUtf8(error.what()));
          }
          updateControls();
        });
    command.request_id = request.request_id;
  }
  catch (const std::exception &error)
  {
    command.pending = false;
    response_label_->setText(QString::fromUtf8(error.what()));
    updateControls();
  }
}

void MtcPanel::updateStatus(const Status &status)
{
  status_ = status;
  status_received_ = true;
  const auto phase = status.phase < phase_names.size() ? phase_names[status.phase] : "未知状态";
  status_label_->setText(QString("任务 %1 · 阶段 %2 · %3 · %4")
      .arg(static_cast<qulonglong>(status.task_id)).arg(status.stage)
      .arg(phase).arg(QString::fromStdString(status.execution_mode)));
  detail_label_->setText(QString::fromStdString(status.message));
  updateControls();
}

void MtcPanel::updateControls()
{
  for (auto &command : commands_)
    if (command.pending && (!command.client->service_is_ready() ||
        std::chrono::steady_clock::now() - command.started > std::chrono::seconds(120)))
    {
      command.client->remove_pending_request(command.request_id);
      command.pending = false;
      response_label_->setText("服务断开或等待超过 120 秒；请求等待已清除，后台任务不会自动取消。请查看任务状态，必要时点击取消任务。");
    }
  const bool pending = std::any_of(commands_.begin(), commands_.end(),
      [](const Command &command) { return command.pending; });
  const bool working = status_.phase == Status::PLANNING || status_.phase == Status::EXECUTING;
  const bool waiting = status_.phase == Status::WAITING_NEXT;
  const bool can_plan = status_received_ && !pending && !working && !waiting;
  const std::array<bool, CommandCount> allowed{
      can_plan, can_plan, can_plan && status_.object_attached,
      status_received_ && !pending && status_.phase == Status::READY,
      status_received_ && !pending && waiting && status_.execution_mode == "step",
      pending || working || waiting || (status_received_ && status_.phase == Status::READY),
      status_received_ && !pending && !working && status_.phase != Status::IDLE};
  for (size_t i = 0; i < commands_.size(); ++i)
  {
    const auto &command = commands_[i];
    command.button->setEnabled(allowed[i] && !command.pending && command.client &&
                              command.client->service_is_ready());
  }
}
} // namespace sim_controller

PLUGINLIB_EXPORT_CLASS(sim_controller::MtcPanel, rviz_common::Panel)
