#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <optional>
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
    poll_interval_ = declare_parameter("controller_poll_interval", .5);
    request_timeout_ = declare_parameter("controller_request_timeout", 3.0);
    names_ = declare_parameter("controllers", std::vector<std::string>{"arm_controller", "gripper_controller"});
    if (!std::isfinite(timeout_) || timeout_ <= 0 || !std::isfinite(poll_interval_) ||
        poll_interval_ <= 0 || names_.empty())
      throw std::invalid_argument("控制器列表及状态超时无效");
    // 客户端等待期限须大于服务端的一秒切换期限，给响应传输留出时间。
    if (!std::isfinite(request_timeout_) || request_timeout_ <= 1.0)
      throw std::invalid_argument("controller_request_timeout 须为大于 1 秒的有限数值");
    list_ = create_client<List>("/controller_manager/list_controllers");
    switch_ = create_client<Switch>("/controller_manager/switch_controller");
    sub_ = create_subscription<msg::HardwareStatus>(
        "/serial_driver/status", rclcpp::QoS(1).reliable().transient_local(),
        [this](msg::HardwareStatus::ConstSharedPtr m) {
          if (!status_ || status_->control_epoch != m->control_epoch)
          {
            reset_ = true;
            urgent_ = true;
          }
          status_ = m; received_at_ = std::chrono::steady_clock::now();
        });
    timer_ = create_wall_timer(std::chrono::milliseconds(50), [this] { update(); });
  }
private:
  using List = controller_manager_msgs::srv::ListControllers;
  using Switch = controller_manager_msgs::srv::SwitchController;
  enum class RequestKind { List, Switch };

  // 所有回调使用默认互斥回调组；请求代次防止超时后的旧响应覆盖新请求状态。
  uint64_t beginRequest(RequestKind kind)
  {
    pending_ = true;
    request_kind_ = kind;
    request_id_.reset();
    request_deadline_ = std::chrono::steady_clock::now() +
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<double>(request_timeout_));
    return ++request_generation_;
  }
  bool finishRequest(uint64_t generation)
  {
    if (!pending_ || generation != request_generation_) return false;
    pending_ = false;
    request_id_.reset();
    return true;
  }
  void recoverRequest(const char *reason)
  {
    // 移除 rclcpp 内部等待项，避免丢失响应造成永久 pending 和等待项累积。
    if (request_id_)
    {
      if (request_kind_ == RequestKind::List) list_->remove_pending_request(*request_id_);
      else switch_->remove_pending_request(*request_id_);
    }
    RCLCPP_WARN(get_logger(), "%s 请求未完成：%s；重新查询实际控制器状态",
                request_kind_ == RequestKind::List ? "list_controllers" : "switch_controller", reason);
    pending_ = false;
    request_id_.reset();
    ++request_generation_;
    urgent_ = false;
    next_poll_ = std::chrono::steady_clock::now() +
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<double>(poll_interval_));
    // 切换超时不代表服务端没有执行；下一轮先查询，不能直接重发旧切换请求。
  }
  bool allowed() const
  {
    return status_ && status_->feedback_valid && !status_->faulted && status_->mode == 2 &&
           !(status_->gripper_state & 6) &&
           std::chrono::duration<double>(std::chrono::steady_clock::now() - received_at_).count() <= timeout_ &&
           std::abs((now() - rclcpp::Time(status_->header.stamp)).seconds()) <= timeout_;
  }
  void update()
  {
    const bool enable_now = allowed();
    if (!last_allowed_ || *last_allowed_ != enable_now) urgent_ = true;
    last_allowed_ = enable_now;
    if (!enable_now) reset_ = true;
    const auto now_steady = std::chrono::steady_clock::now();
    if (pending_)
    {
      if (now_steady < request_deadline_) return;
      recoverRequest("响应等待超时");
    }
    if (!list_->service_is_ready() || !switch_->service_is_ready()) return;
    if (!urgent_ && now_steady < next_poll_) return;
    urgent_ = false;
    next_poll_ = now_steady + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                                  std::chrono::duration<double>(poll_interval_));
    const auto generation = beginRequest(RequestKind::List);
    try
    {
      const auto future = list_->async_send_request(std::make_shared<List::Request>(),
          [this, generation](rclcpp::Client<List>::SharedFuture f) {
        if (!finishRequest(generation)) return;
        try
        {
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
          const auto switch_generation = beginRequest(RequestKind::Switch);
          const auto switch_future = switch_->async_send_request(request,
              [this, switch_generation](rclcpp::Client<Switch>::SharedFuture result) {
            if (!finishRequest(switch_generation)) return;
            urgent_ = true;  // 及时确认切换结果，必要时完成清轨后的重新启用。
            try
            {
              if (!result.get()->ok) RCLCPP_WARN(get_logger(), "控制器切换未完成，保持硬件输出门控并重试");
            }
            catch (const std::exception &error) { recoverRequest(error.what()); }
          });
          request_id_ = switch_future.request_id;
        }
        catch (const std::exception &error) { recoverRequest(error.what()); }
      });
      request_id_ = future.request_id;
    }
    catch (const std::exception &error) { recoverRequest(error.what()); }
  }
  double timeout_, poll_interval_, request_timeout_;
  bool pending_ = false, reset_ = true, urgent_ = true;
  RequestKind request_kind_ = RequestKind::List;
  uint64_t request_generation_ = 0;
  std::optional<int64_t> request_id_;
  std::chrono::steady_clock::time_point request_deadline_{};
  std::optional<bool> last_allowed_;
  std::chrono::steady_clock::time_point next_poll_{};
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
