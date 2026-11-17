#include "serial_driver/engineer_hardware.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <pluginlib/class_list_macros.hpp>
#include <sstream>
#include <stdexcept>

namespace serial_driver
{
namespace
{
constexpr unsigned kAllJoints = (1u << kJoints) - 1;
const std::array<std::string, kJoints> kNames{
    "link_1_joint", "link_2_joint", "link_3_joint", "link_4_joint",
    "link_5_joint", "link_6_joint", "gripper_state_joint"};
using Callback = hardware_interface::CallbackReturn;
using Result = hardware_interface::return_type;
}
EngineerHardware::~EngineerHardware() { stopWorker(); }
Callback EngineerHardware::on_init(const hardware_interface::HardwareInfo &info)
{
  if (SystemInterface::on_init(info) != Callback::SUCCESS) return Callback::ERROR;
  try
  {
    if (info_.joints.size() != kJoints) throw std::invalid_argument("需要六轴和一个夹爪虚拟关节");
    for (size_t i = 0; i < kJoints; ++i)
    {
      const auto &joint = info_.joints[i];
      if (joint.name != kNames[i] || joint.command_interfaces.size() != 1 ||
          joint.command_interfaces[0].name != "position" || joint.state_interfaces.size() != 2 ||
          joint.state_interfaces[0].name != "position" || joint.state_interfaces[1].name != "velocity")
        throw std::invalid_argument("关节顺序或 position/velocity 接口不正确: " + joint.name);
      lower_[i] = std::stod(joint.command_interfaces[0].min);
      upper_[i] = std::stod(joint.command_interfaces[0].max);
      initial_[i] = std::stod(joint.state_interfaces[0].initial_value);
      const double minimum = i == 6 ? 0. : -kPi;
      const double maximum = i == 6 ? kGripperOpen : kPi;
      if (!std::isfinite(lower_[i]) || !std::isfinite(upper_[i]) ||
          lower_[i] < minimum - 1e-10 || upper_[i] > maximum + 1e-10 || lower_[i] >= upper_[i])
        throw std::invalid_argument("关节限位须处于协议范围内: " + joint.name);
    }
    auto parameter = [&](const std::string &name, const std::string &fallback) {
      const auto found = info_.hardware_parameters.find(name);
      return found == info_.hardware_parameters.end() ? fallback : found->second;
    };
    const auto mock = parameter("use_mock_hardware", "true");
    if (mock != "true" && mock != "false") throw std::invalid_argument("use_mock_hardware 须为 true/false");
    mock_ = mock == "true";
    port_name_ = parameter("serial_port", "/dev/ttyUSB0");
    baud_ = std::stoi(parameter("baud_rate", "115200"));
    receive_timeout_ = std::stod(parameter("receive_timeout_ms", "100")) / 1000.;
    startup_timeout_ = std::stod(parameter("startup_timeout_ms", "3000")) / 1000.;
    tx_rate_ = std::stod(parameter("tx_rate", "20"));
    feedback_rate_ = std::stod(parameter("feedback_rate", "50"));
    const int mode = std::stoi(parameter("mock_mode", "2"));
    if (mode < 0 || mode > 255) throw std::invalid_argument("mock_mode 须在 0～255 内");
    mock_mode_ = static_cast<uint8_t>(mode);
    for (double v : {receive_timeout_, startup_timeout_, tx_rate_, feedback_rate_})
      if (!std::isfinite(v) || v <= 0) throw std::invalid_argument("超时及频率须为有限正数");
    if (tx_rate_ > 100 || feedback_rate_ > 100 || receive_timeout_ < .02)
      throw std::invalid_argument("收发频率不得超过 100Hz，接收超时不得低于 20ms");
    if (mock_) validatePositions(initial_);
    position_.fill(std::numeric_limits<double>::quiet_NaN());
    velocity_.fill(0.);
    command_ = position_;
    node_ = std::make_shared<rclcpp::Node>("serial_hardware", rclcpp::NodeOptions().use_global_arguments(false));
    const auto qos = rclcpp::QoS(1).reliable().transient_local();
    status_pub_ = node_->create_publisher<msg::HardwareStatus>("/serial_driver/status", qos);
    mode_pub_ = node_->create_publisher<std_msgs::msg::UInt8>("/serial_driver/mode", qos);
    if (mock_)
      mock_sub_ = node_->create_subscription<std_msgs::msg::UInt8>(
          "/serial_driver/mock_mode", rclcpp::QoS(1).reliable(),
          [this](std_msgs::msg::UInt8::ConstSharedPtr m) { mock_mode_ = m->data; });
    return Callback::SUCCESS;
  }
  catch (const std::exception &e)
  {
    RCLCPP_ERROR(rclcpp::get_logger("serial_driver"), "初始化失败: %s", e.what());
    return Callback::ERROR;
  }
}
void EngineerHardware::validatePositions(const JointValues &values) const
{
  for (size_t i = 0; i < kJoints; ++i)
    if (!std::isfinite(values[i]) || values[i] < lower_[i] - 1e-10 || values[i] > upper_[i] + 1e-10)
      throw std::invalid_argument("关节位置无效或超出配置限位: " + kNames[i]);
}
Callback EngineerHardware::on_configure(const rclcpp_lifecycle::State &)
{
  stopWorker();
  try
  {
    if (!mock_) port_.open(port_name_, baud_);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      active_ = received_ = faulted_ = require_stop_ = false;
      claimed_ = 0;
      error_.clear(); parser_.clear();
      rx_frames_ = tx_frames_ = 0;
      feedback_stamp_ = builtin_interfaces::msg::Time();
      feedback_ = Feedback();
      feedback_.position = initial_;
      target_ = initial_;
    }
    running_ = true;
    worker_ = std::thread(&EngineerHardware::ioLoop, this);
    RCLCPP_INFO(node_->get_logger(), "硬件=%s port=%s baud=%d",
                mock_ ? "mock" : "serial", port_name_.c_str(), baud_);
    return Callback::SUCCESS;
  }
  catch (const std::exception &e)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    failLocked(e.what());
    return Callback::ERROR;
  }
}
Callback EngineerHardware::on_activate(const rclcpp_lifecycle::State &)
{
  std::unique_lock<std::mutex> lock(mutex_);
  const bool ready = wake_.wait_for(lock, std::chrono::duration<double>(startup_timeout_),
                                   [this] { return received_ || faulted_ || !running_; });
  if (!ready || !freshLocked())
  {
    failLocked("启动时未收到有效状态帧，未发送默认零位目标");
    return Callback::ERROR;
  }
  position_ = feedback_.position; velocity_ = feedback_.velocity;
  command_ = target_ = position_;
  read_epoch_ = target_epoch_ = epoch_;
  active_ = true;
  return Callback::SUCCESS;
}
Callback EngineerHardware::on_deactivate(const rclcpp_lifecycle::State &)
{
  std::lock_guard<std::mutex> lock(mutex_);
  active_ = false; claimed_ = 0; require_stop_ = true;
  return Callback::SUCCESS;
}
Callback EngineerHardware::on_cleanup(const rclcpp_lifecycle::State &)
{
  stopWorker(); return Callback::SUCCESS;
}
Callback EngineerHardware::on_error(const rclcpp_lifecycle::State &)
{
  std::lock_guard<std::mutex> lock(mutex_);
  failLocked(error_.empty() ? "ros2_control 硬件错误" : error_);
  // 保留监测线程上报故障；不自动重连或恢复旧任务。
  return Callback::SUCCESS;
}
void EngineerHardware::stopWorker()
{
  running_ = false; wake_.notify_all();
  if (worker_.joinable()) worker_.join();
  port_.close();
}
std::vector<hardware_interface::StateInterface> EngineerHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> result;
  for (size_t i = 0; i < kJoints; ++i)
  {
    result.emplace_back(kNames[i], "position", &position_[i]);
    result.emplace_back(kNames[i], "velocity", &velocity_[i]);
  }
  return result;
}
std::vector<hardware_interface::CommandInterface> EngineerHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> result;
  for (size_t i = 0; i < kJoints; ++i) result.emplace_back(kNames[i], "position", &command_[i]);
  return result;
}
bool EngineerHardware::freshLocked() const
{
  return received_ && !faulted_ &&
         std::chrono::duration<double>(Clock::now() - last_rx_).count() <= receive_timeout_;
}
bool EngineerHardware::enabledLocked() const
{
  return active_ && freshLocked() && feedback_.mode == 2 && !(feedback_.grip_state & 6) &&
         claimed_ == kAllJoints && !require_stop_;
}
unsigned EngineerHardware::interfaceMask(const std::vector<std::string> &names) const
{
  unsigned mask = 0;
  for (size_t i = 0; i < kJoints; ++i)
    if (std::find(names.begin(), names.end(), kNames[i] + "/position") != names.end()) mask |= 1u << i;
  return mask;
}
Result EngineerHardware::prepare_command_mode_switch(const std::vector<std::string> &start,
                                                    const std::vector<std::string> &stop)
{
  std::lock_guard<std::mutex> lock(mutex_);
  const unsigned starting = interfaceMask(start), remaining = claimed_ & ~interfaceMask(stop);
  if (starting && (!active_ || !freshLocked() || feedback_.mode != 2 ||
                   (feedback_.grip_state & 6) || (require_stop_ && remaining))) return Result::ERROR;
  return Result::OK;
}
Result EngineerHardware::perform_command_mode_switch(const std::vector<std::string> &start,
                                                    const std::vector<std::string> &stop)
{
  std::lock_guard<std::mutex> lock(mutex_);
  claimed_ &= ~interfaceMask(stop);
  if (!claimed_) require_stop_ = false;
  const unsigned starting = interfaceMask(start);
  if (starting)
  {
    if (!active_ || !freshLocked() || feedback_.mode != 2 || (feedback_.grip_state & 6) || require_stop_)
      return Result::ERROR;
    for (size_t i = 0; i < kJoints; ++i)
      if (starting & (1u << i)) command_[i] = feedback_.position[i];
    claimed_ |= starting;
    target_ = command_;
    read_epoch_ = target_epoch_ = epoch_;
  }
  return Result::OK;
}
Result EngineerHardware::read(const rclcpp::Time &, const rclcpp::Duration &)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (!freshLocked()) return Result::ERROR;
  position_ = feedback_.position; velocity_ = feedback_.velocity;
  if (read_epoch_ != epoch_ || feedback_.mode != 2 || require_stop_)
  {
    command_ = position_;
    read_epoch_ = epoch_;
  }
  return Result::OK;
}
Result EngineerHardware::write(const rclcpp::Time &, const rclcpp::Duration &)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (!freshLocked()) return Result::ERROR;
  if (!enabledLocked() || read_epoch_ != epoch_) return Result::OK;
  try
  {
    validatePositions(command_);
    encodeCommand(command_);
    target_ = command_; target_epoch_ = epoch_;
    return Result::OK;
  }
  catch (const std::exception &e) { failLocked(e.what()); return Result::ERROR; }
}
void EngineerHardware::failLocked(const std::string &why)
{
  if (!faulted_) RCLCPP_ERROR(node_ ? node_->get_logger() : rclcpp::get_logger("serial_driver"), "%s", why.c_str());
  faulted_ = true; require_stop_ = true; error_ = why;
  wake_.notify_all();
}
void EngineerHardware::acceptFeedback(const Feedback &f)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (faulted_) return;
  try { validatePositions(f.position); }
  catch (const std::exception &e) { failLocked(e.what()); return; }
  if (!received_ || f.mode != feedback_.mode || ((f.grip_state ^ feedback_.grip_state) & 6))
  {
    ++epoch_;
    require_stop_ = claimed_ != 0;
    target_ = f.position; target_epoch_ = 0;
  }
  feedback_ = f;
  feedback_stamp_ = node_->now();
  last_rx_ = Clock::now();
  received_ = true; ++rx_frames_;
  wake_.notify_all();
}
void EngineerHardware::publishStatus()
{
  msg::HardwareStatus status;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    status.header.stamp = feedback_stamp_;
    status.use_mock_hardware = mock_;
    status.feedback_valid = freshLocked(); status.faulted = faulted_;
    status.tx_enabled = enabledLocked();
    status.mode = received_ ? feedback_.mode : 0;
    status.gripper_position = feedback_.grip; status.gripper_state = feedback_.grip_state;
    status.control_epoch = epoch_;
    status.rx_frames = rx_frames_; status.tx_frames = tx_frames_;
    status.rejected_frames = parser_.rejected(); status.error = error_;
  }
  status_pub_->publish(status);
  std_msgs::msg::UInt8 mode;
  mode.data = status.feedback_valid ? status.mode : 0;
  mode_pub_->publish(mode);
}
void EngineerHardware::ioLoop()
{
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node_);
  auto next_tx = Clock::now(), next_feedback = next_tx, next_status = next_tx;
  const auto tx_period = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1 / tx_rate_));
  const auto feedback_period = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1 / feedback_rate_));
  Feedback mock_feedback;
  mock_feedback.position = initial_;
  if (mock_) mock_feedback.grip = encodeCommand(initial_)[14];
  try
  {
    while (running_ && rclcpp::ok())
    {
      executor.spin_some(std::chrono::milliseconds(1));
      const auto now = Clock::now();
      if (mock_ && now >= next_feedback)
      {
        mock_feedback.mode = mock_mode_; mock_feedback.grip_state = 1;
        acceptFeedback(mock_feedback);
        next_feedback = now + feedback_period;
      }
      if (!mock_)
      {
        const auto data = port_.readAvailable();
        for (const auto &feedback : parser_.append(data.data(), data.size())) acceptFeedback(feedback);
      }
      TxFrame frame{};
      bool send = false;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (received_ && !faulted_ && !freshLocked()) failLocked("有效反馈超时，禁止输出；需重启硬件链路");
        if (now >= next_tx)
        {
          send = enabledLocked() && target_epoch_ == epoch_;
          if (send) frame = encodeCommand(target_);
          next_tx = now + tx_period;
        }
      }
      if (send)
      {
        if (mock_)
        {
          // 模拟也经过相同的协议编码，保留 0.01° 与 uint8 开合量的量化。
          std::array<uint8_t, kRxSize> echo{};
          echo[0] = 0x55; echo[1] = 0xaa;
          std::copy(frame.begin() + 2, frame.begin() + 14, echo.begin() + 2);
          echo[26] = frame[14]; echo[27] = 1; echo[28] = mock_mode_;
          echo[29] = checksum(echo.data() + 2, 27); echo[30] = 0x0d;
          decodeFeedback(echo.data(), mock_feedback);
        }
        else port_.writeFrame(frame.data(), frame.size());
        std::lock_guard<std::mutex> lock(mutex_);
        ++tx_frames_;
      }
      if (now >= next_status) { publishStatus(); next_status = now + feedback_period; }
      std::unique_lock<std::mutex> lock(mutex_);
      wake_.wait_for(lock, std::chrono::milliseconds(2), [this] { return !running_; });
    }
  }
  catch (const std::exception &e)
  {
    { std::lock_guard<std::mutex> lock(mutex_); failLocked(e.what()); }
    // I/O 故障锁存，不再读写端口，持续发布离线状态。
    while (running_ && rclcpp::ok())
    {
      publishStatus();
      std::unique_lock<std::mutex> lock(mutex_);
      wake_.wait_for(lock, std::chrono::milliseconds(20), [this] { return !running_; });
    }
  }
  executor.remove_node(node_);
}
} // namespace serial_driver
PLUGINLIB_EXPORT_CLASS(serial_driver::EngineerHardware, hardware_interface::SystemInterface)
