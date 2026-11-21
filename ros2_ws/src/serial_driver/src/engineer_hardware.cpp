#include "serial_driver/engineer_hardware.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <list>
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
EngineerHardware::~EngineerHardware() { stopWorkers(); }
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
std::string EngineerHardware::openAutomaticPort()
{
  constexpr size_t required_frames = 3;
  struct Candidate
  {
    std::string path;
    std::string error;
    SerialPort port;
    FeedbackParser parser;
    size_t valid_frames = 0;
    Clock::time_point last_valid{};
  };

  std::list<Candidate> candidates;
  for (const auto &device : std::filesystem::directory_iterator("/dev"))
  {
    const auto name = device.path().filename().string();
    if ((name.compare(0, 6, "ttyACM") != 0 && name.compare(0, 6, "ttyUSB") != 0) ||
        name.size() <= 6 || name.find_first_not_of("0123456789", 6) != std::string::npos)
      continue;
    auto &candidate = candidates.emplace_back();
    candidate.path = device.path().string();
    try { candidate.port.open(candidate.path, baud_, false); }
    catch (const std::exception &e) { candidate.error = e.what(); }
  }
  if (candidates.empty())
    throw std::runtime_error("串口自动识别：未找到 /dev/ttyACM* 或 /dev/ttyUSB*");

  const auto deadline = Clock::now() + std::chrono::duration<double>(startup_timeout_);
  const auto max_feedback_gap = std::chrono::duration<double>(receive_timeout_);
  while (rclcpp::ok() && Clock::now() < deadline)
  {
    bool listening = false;
    for (auto &candidate : candidates)
    {
      if (!candidate.error.empty()) continue;
      try
      {
        const auto data = candidate.port.readAvailable();
        const auto rejected = candidate.parser.rejected();
        const auto frames = candidate.parser.append(data.data(), data.size());
        const auto now = Clock::now();
        if (now - candidate.last_valid > max_feedback_gap || rejected != candidate.parser.rejected())
          candidate.valid_frames = 0;
        if (rejected == candidate.parser.rejected() && !frames.empty())
        {
          candidate.valid_frames = std::min(required_frames, candidate.valid_frames + frames.size());
          candidate.last_valid = now;
        }
        listening = true;
      }
      catch (const std::exception &e)
      {
        candidate.error = e.what();
        candidate.port.close();
      }
    }
    if (!listening) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  if (!rclcpp::ok()) throw std::runtime_error("串口自动识别已取消");

  Candidate *selected = nullptr;
  size_t matches = 0;
  std::ostringstream details;
  const auto now = Clock::now();
  for (auto &candidate : candidates)
  {
    details << "\n  " << candidate.path << ": ";
    if (!candidate.error.empty())
      details << candidate.error;
    else if (candidate.valid_frames == required_frames && now - candidate.last_valid <= max_feedback_gap)
    {
      details << "匹配 27 协议";
      selected = &candidate;
      ++matches;
    }
    else
      details << "未收到连续有效的 27 协议反馈帧";
  }
  if (matches != 1)
    throw std::runtime_error("串口自动识别未找到唯一匹配设备" + details.str());

  // 保留已验证的连接，避免按名称重开时设备编号已被复用。
  port_.swap(selected->port);
  return selected->path;
}
Callback EngineerHardware::on_configure(const rclcpp_lifecycle::State &)
{
  stopWorkers();
  try
  {
    auto device = port_name_;
    if (!mock_)
    {
      if (device == "auto")
        device = openAutomaticPort();
      else
        port_.open(device, baud_);
    }
    {
      std::lock_guard<std::mutex> lock(mutex_);
      active_ = received_ = faulted_ = require_stop_ = false;
      claimed_ = 0;
      error_.clear(); parser_.clear();
      rx_frames_ = tx_frames_ = rejected_frames_ = 0;
      feedback_stamp_ = builtin_interfaces::msg::Time();
      feedback_ = Feedback();
      feedback_.position = initial_;
      target_ = initial_;
      if (mock_)
      {
        mock_feedback_ = Feedback();
        mock_feedback_.position = initial_;
        mock_feedback_.grip = encodeCommand(initial_)[14];
        mock_feedback_.grip_state = 1;
        mock_feedback_.mode = mock_mode_.load();
        const auto frame = encodeFeedback(mock_feedback_);
        receiveBytesLocked(frame.data(), frame.size());
        last_mock_tx_ = Clock::time_point();
      }
    }
    running_ = true;
    if (!mock_)
    {
      receive_worker_ = std::thread(&EngineerHardware::receiveLoop, this);
      transmit_worker_ = std::thread(&EngineerHardware::transmitLoop, this);
    }
    status_worker_ = std::thread(&EngineerHardware::statusLoop, this);
    RCLCPP_INFO(node_->get_logger(), "硬件=%s port=%s baud=%d",
                mock_ ? "mock" : "serial", device.c_str(), baud_);
    return Callback::SUCCESS;
  }
  catch (const std::exception &e)
  {
    stopWorkers();
    RCLCPP_ERROR(node_->get_logger(), "%s", e.what());
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
  stopWorkers(); return Callback::SUCCESS;
}
Callback EngineerHardware::on_error(const rclcpp_lifecycle::State &)
{
  std::lock_guard<std::mutex> lock(mutex_);
  failLocked(error_.empty() ? "ros2_control 硬件错误" : error_);
  // 保留监测线程上报故障；不自动重连或恢复旧任务。
  return Callback::SUCCESS;
}
void EngineerHardware::stopWorkers()
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
  }
  wake_.notify_all();
  if (receive_worker_.joinable()) receive_worker_.join();
  if (transmit_worker_.joinable()) transmit_worker_.join();
  if (status_worker_.joinable()) status_worker_.join();
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
         (mock_ || std::chrono::duration<double>(Clock::now() - last_rx_).count() <= receive_timeout_);
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
  // 模拟反馈随控制循环生成，不依赖 ROS 通信线程的调度，不套用串口掉线判定。
  if (mock_ && !faulted_ &&
      (mock_feedback_.mode != mock_mode_.load() ||
       std::chrono::duration<double>(Clock::now() - last_rx_).count() >= 1 / feedback_rate_))
  {
    try
    {
      mock_feedback_.mode = mock_mode_.load();
      const auto frame = encodeFeedback(mock_feedback_);
      receiveBytesLocked(frame.data(), frame.size());
    }
    catch (const std::exception &e) { failLocked(e.what()); return Result::ERROR; }
  }
  if (!freshLocked())
  {
    if (!faulted_) failLocked("读取时有效串口反馈已超时");
    return Result::ERROR;
  }
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
  if (!freshLocked())
  {
    if (!faulted_) failLocked("发送前有效串口反馈已超时");
    return Result::ERROR;
  }
  if (!enabledLocked() || read_epoch_ != epoch_) return Result::OK;
  try
  {
    validatePositions(command_);
    const auto frame = encodeCommand(command_);
    target_ = command_; target_epoch_ = epoch_;
    if (mock_ && mock_mode_.load() == 2 &&
        std::chrono::duration<double>(Clock::now() - last_mock_tx_).count() >= 1 / tx_rate_)
    {
      // 虚拟 MCU 在 write() 采纳目标，下一次 read() 返回量化后的实际状态。
      JointValues actual{};
      if (!decodeCommand(frame.data(), actual))
        throw std::runtime_error("模拟 MCU 拒绝无效控制帧");
      mock_feedback_.position = actual;
      mock_feedback_.grip = frame[14];
      mock_feedback_.grip_state = 1;
      mock_feedback_.mode = mock_mode_.load();
      last_mock_tx_ = Clock::now();
      ++tx_frames_;
    }
    return Result::OK;
  }
  catch (const std::exception &e) { failLocked(e.what()); return Result::ERROR; }
}
void EngineerHardware::failLocked(const std::string &why)
{
  if (!faulted_) error_ = why;
  faulted_ = true; require_stop_ = true;
  wake_.notify_all();
}
void EngineerHardware::receiveBytesLocked(const uint8_t *data, size_t size)
{
  for (const auto &feedback : parser_.append(data, size)) acceptFeedbackLocked(feedback);
  rejected_frames_ = parser_.rejected();
}
void EngineerHardware::acceptFeedbackLocked(const Feedback &f)
{
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
void EngineerHardware::publishStatus(bool &fault_reported)
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
    status.rejected_frames = rejected_frames_; status.error = error_;
  }
  if (status.faulted && !fault_reported)
  {
    RCLCPP_ERROR(node_->get_logger(), "%s", status.error.c_str());
    fault_reported = true;
  }
  status_pub_->publish(status);
  std_msgs::msg::UInt8 mode;
  mode.data = status.feedback_valid ? status.mode : 0;
  mode_pub_->publish(mode);
}
void EngineerHardware::receiveLoop()
{
  try
  {
    while (running_ && rclcpp::ok())
    {
      // 真实串口由接收线程独占解析器。只在提交反馈与计数时持锁。
      const auto bytes = port_.readAvailable(5);
      const auto frames = parser_.append(bytes.data(), bytes.size());
      std::lock_guard<std::mutex> lock(mutex_);
      rejected_frames_ = parser_.rejected();
      for (const auto &feedback : frames) acceptFeedbackLocked(feedback);
      if (received_ && !faulted_ && !freshLocked())
        failLocked("有效反馈超时，禁止输出；需重启硬件链路");
    }
  }
  catch (const std::exception &e)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    failLocked(e.what());
  }
}
void EngineerHardware::transmitLoop()
{
  const auto period = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1 / tx_rate_));
  try
  {
    while (running_ && rclcpp::ok())
    {
      const auto deadline = Clock::now() + period;
      TxFrame frame{};
      bool send;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        send = running_ && enabledLocked() && target_epoch_ == epoch_;
        if (send) frame = encodeCommand(target_);
      }
      if (send)
      {
        // 串口写入可能等待缓冲区，不能占用反馈状态锁。
        port_.writeFrame(frame.data(), frame.size());
        std::lock_guard<std::mutex> lock(mutex_);
        ++tx_frames_;
      }
      std::unique_lock<std::mutex> lock(mutex_);
      wake_.wait_until(lock, deadline, [this] { return !running_; });
    }
  }
  catch (const std::exception &e)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    failLocked(e.what());
  }
}
void EngineerHardware::statusLoop()
{
  const auto period = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1 / feedback_rate_));
  bool fault_reported = false;
  try
  {
    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node_);
    while (running_ && rclcpp::ok())
    {
      const auto deadline = Clock::now() + period;
      executor.spin_some(std::chrono::milliseconds(1));
      publishStatus(fault_reported);
      std::unique_lock<std::mutex> lock(mutex_);
      wake_.wait_until(lock, deadline, [this] { return !running_; });
    }
    executor.remove_node(node_);
  }
  catch (const std::exception &e)
  {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      failLocked(e.what());
    }
    RCLCPP_ERROR(node_->get_logger(), "硬件状态线程异常: %s", e.what());
  }
}
} // namespace serial_driver
PLUGINLIB_EXPORT_CLASS(serial_driver::EngineerHardware, hardware_interface::SystemInterface)
