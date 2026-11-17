#include <algorithm>
#include <chrono>
#include <cmath>
#include <controller_manager_msgs/srv/list_controllers.hpp>
#include <controller_manager_msgs/srv/switch_controller.hpp>
#include <rclcpp/rclcpp.hpp>
#include <serial_driver/msg/hardware_status.hpp>

namespace serial_driver
{
// MCU 控制权变化时，停用轨迹控制器以清除旧轨迹；重新允许自动控制后再激活。
// 不负责调用 MTC 的 plan/execute，不把 mode=2 的每一帧当作新的任务请求。
class HardwareSupervisor : public rclcpp::Node
{
public:
  HardwareSupervisor() : Node("hardware_supervisor")
  {
    timeout_ = declare_parameter("status_timeout", .1);
    names_ = declare_parameter("controllers", std::vector<std::string>{"arm_controller", "gripper_controller"});
    if (!std::isfinite(timeout_) || timeout_ <= 0 || names_.empty())
      throw std::invalid_argument("控制器列表及状态超时无效");
    list_ = create_client<List>("/controller_manager/list_controllers");
    switch_ = create_client<Switch>("/controller_manager/switch_controller");
    sub_ = create_subscription<msg::HardwareStatus>(
        "/serial_driver/status", rclcpp::QoS(1).reliable().transient_local(),
        [this](msg::HardwareStatus::ConstSharedPtr m) {
          if (!status_ || status_->control_epoch != m->control_epoch) reset_ = true;
          status_ = m; received_at_ = std::chrono::steady_clock::now();
        });
    timer_ = create_wall_timer(std::chrono::milliseconds(50), [this] { update(); });
  }
private:
  using List = controller_manager_msgs::srv::ListControllers;
  using Switch = controller_manager_msgs::srv::SwitchController;
  bool allowed() const
  {
    return status_ && status_->feedback_valid && !status_->faulted && status_->mode == 2 &&
           !(status_->gripper_state & 6) &&
           std::chrono::duration<double>(std::chrono::steady_clock::now() - received_at_).count() <= timeout_ &&
           std::abs((now() - rclcpp::Time(status_->header.stamp)).seconds()) <= timeout_;
  }
  void update()
  {
    if (!allowed()) reset_ = true;
    if (pending_ || !list_->service_is_ready() || !switch_->service_is_ready()) return;
    pending_ = true;
    list_->async_send_request(std::make_shared<List::Request>(), [this](rclcpp::Client<List>::SharedFuture f) {
      pending_ = false;
      const auto response = f.get();
      std::vector<std::string> active;
      for (const auto &name : names_)
      {
        const auto it = std::find_if(response->controller.begin(), response->controller.end(),
                                     [&](const auto &c) { return c.name == name; });
        if (it == response->controller.end() || (it->state != "active" && it->state != "inactive")) return;
        if (it->state == "active") active.push_back(name);
      }
      const bool enable = allowed();
      auto request = std::make_shared<Switch::Request>();
      request->strictness = Switch::Request::STRICT;
      request->timeout.sec = 1;
      if ((!enable || reset_) && !active.empty()) request->deactivate_controllers = active;
      else
      {
        if (active.empty()) reset_ = false;
        if (!enable) return;
        for (const auto &name : names_)
          if (std::find(active.begin(), active.end(), name) == active.end()) request->activate_controllers.push_back(name);
        if (request->activate_controllers.empty()) return;
      }
      pending_ = true;
      switch_->async_send_request(request, [this](rclcpp::Client<Switch>::SharedFuture result) {
        pending_ = false;
        if (!result.get()->ok) RCLCPP_WARN(get_logger(), "控制器切换未完成，保持硬件输出门控并重试");
      });
    });
  }
  double timeout_;
  bool pending_ = false, reset_ = true;
  std::vector<std::string> names_;
  std::chrono::steady_clock::time_point received_at_{};
  msg::HardwareStatus::ConstSharedPtr status_;
  rclcpp::Client<List>::SharedPtr list_;
  rclcpp::Client<Switch>::SharedPtr switch_;
  rclcpp::Subscription<msg::HardwareStatus>::SharedPtr sub_;
  rclcpp::TimerBase::SharedPtr timer_;
};
} // namespace serial_driver
int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<serial_driver::HardwareSupervisor>());
  rclcpp::shutdown();
  return 0;
}
