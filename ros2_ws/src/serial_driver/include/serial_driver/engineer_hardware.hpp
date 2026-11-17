#pragma once
#include "serial_driver/protocol.hpp"
#include "serial_driver/serial_port.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <hardware_interface/system_interface.hpp>
#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include <serial_driver/msg/hardware_status.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <thread>

namespace serial_driver
{
class EngineerHardware : public hardware_interface::SystemInterface
{
public:
  ~EngineerHardware() override;
  hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo &) override;
  hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &) override;
  hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State &) override;
  hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override;
  hardware_interface::CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override;
  hardware_interface::CallbackReturn on_error(const rclcpp_lifecycle::State &) override;
  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;
  hardware_interface::return_type prepare_command_mode_switch(
      const std::vector<std::string> &, const std::vector<std::string> &) override;
  hardware_interface::return_type perform_command_mode_switch(
      const std::vector<std::string> &, const std::vector<std::string> &) override;
  hardware_interface::return_type read(const rclcpp::Time &, const rclcpp::Duration &) override;
  hardware_interface::return_type write(const rclcpp::Time &, const rclcpp::Duration &) override;
private:
  using Clock = std::chrono::steady_clock;
  bool freshLocked() const;
  bool enabledLocked() const;
  unsigned interfaceMask(const std::vector<std::string> &) const;
  void stopWorker();
  void ioLoop();
  void acceptFeedback(const Feedback &);
  void publishStatus();
  void failLocked(const std::string &);
  void validatePositions(const JointValues &) const;

  JointValues position_{}, velocity_{}, command_{}, lower_{}, upper_{}, initial_{};
  JointValues target_{};
  bool mock_ = true, active_ = false, received_ = false, faulted_ = false, require_stop_ = false;
  unsigned claimed_ = 0;
  uint8_t mock_mode_ = 2;
  double receive_timeout_ = .1, startup_timeout_ = 3., tx_rate_ = 20., feedback_rate_ = 50.;
  int baud_ = 115200;
  std::string port_name_ = "/dev/ttyUSB0", error_;
  Feedback feedback_;
  uint64_t epoch_ = 0, read_epoch_ = 0, target_epoch_ = 0, rx_frames_ = 0, tx_frames_ = 0;
  Clock::time_point last_rx_{};
  builtin_interfaces::msg::Time feedback_stamp_;
  SerialPort port_;
  FeedbackParser parser_;
  std::mutex mutex_;
  std::condition_variable wake_;
  std::atomic<bool> running_{false};
  std::thread worker_;
  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<msg::HardwareStatus>::SharedPtr status_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr mode_pub_;
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr mock_sub_;
};
} // namespace serial_driver
