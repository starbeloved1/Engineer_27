#include "serial_driver/protocol.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace serial_driver
{
namespace
{
int readSigned(const uint8_t *p)
{
  const int value = (int(p[0]) << 8) | p[1];
  return value >= 32768 ? value - 65536 : value;
}
void writeSigned(uint8_t *p, int value)
{
  const auto raw = static_cast<uint16_t>(static_cast<int16_t>(value));
  p[0] = static_cast<uint8_t>(raw >> 8);
  p[1] = static_cast<uint8_t>(raw & 0xff);
}
}
uint8_t checksum(const uint8_t *data, size_t size)
{
  unsigned sum = 0;
  for (size_t i = 0; i < size; ++i) sum += data[i];
  return static_cast<uint8_t>(sum & 0xff);
}
TxFrame encodeCommand(const JointValues &position)
{
  TxFrame frame{};
  frame[0] = 0xaa; frame[1] = 0x55;
  for (size_t i = 0; i < kArmJoints; ++i)
  {
    const double q = position[i];
    if (!std::isfinite(q) || std::abs(q) > kPi + 1e-10)
      throw std::invalid_argument("关节目标超出协议 ±180°，拒绝整帧，不截断或取模");
    const auto raw = static_cast<uint16_t>(static_cast<int16_t>(std::lround(q / kAngleUnit)));
    frame[2 + 2*i] = static_cast<uint8_t>(raw >> 8);
    frame[3 + 2*i] = static_cast<uint8_t>(raw & 0xff);
  }
  const double q = position[6];
  if (!std::isfinite(q) || q < -1e-10 || q > kGripperOpen + 1e-10)
    throw std::invalid_argument("夹爪虚拟关节目标超出 [0, 0.01]");
  frame[14] = static_cast<uint8_t>(std::lround(255 * (1 - std::clamp(q / kGripperOpen, 0., 1.))));
  frame[15] = checksum(frame.data() + 2, 13);
  frame[16] = 0x0d;
  return frame;
}
bool decodeCommand(const uint8_t *frame, JointValues &position)
{
  if (frame[0] != 0xaa || frame[1] != 0x55 || frame[16] != 0x0d ||
      checksum(frame + 2, 13) != frame[15]) return false;
  JointValues result{};
  for (size_t i = 0; i < kArmJoints; ++i)
  {
    const int raw = readSigned(frame + 2 + 2*i);
    if (raw < -18000 || raw > 18000) return false;
    result[i] = raw * kAngleUnit;
  }
  result[6] = kGripperOpen * (1 - frame[14] / 255.);
  position = result;
  return true;
}
RxFrame encodeFeedback(const Feedback &feedback)
{
  RxFrame frame{};
  frame[0] = 0x55; frame[1] = 0xaa;
  for (size_t i = 0; i < kArmJoints; ++i)
  {
    const double q = feedback.position[i];
    if (!std::isfinite(q) || std::abs(q) > kPi + 1e-10)
      throw std::invalid_argument("反馈位置超出协议 ±180°");
    const double speed = std::round(feedback.velocity[i] / kAngleUnit);
    if (!std::isfinite(speed) || speed < -32768 || speed > 32767)
      throw std::invalid_argument("反馈速度超出协议 int16 范围");
    writeSigned(frame.data() + 2 + 2*i, static_cast<int>(std::lround(q / kAngleUnit)));
    writeSigned(frame.data() + 14 + 2*i, static_cast<int>(speed));
  }
  if (feedback.grip_state & 0xf8)
    throw std::invalid_argument("夹爪状态保留位必须为 0");
  frame[26] = feedback.grip;
  frame[27] = feedback.grip_state;
  frame[28] = feedback.mode;
  frame[29] = checksum(frame.data() + 2, 27);
  frame[30] = 0x0d;
  return frame;
}
bool decodeFeedback(const uint8_t *frame, Feedback &feedback)
{
  if (frame[0] != 0x55 || frame[1] != 0xaa || frame[30] != 0x0d ||
      checksum(frame + 2, 27) != frame[29] || (frame[27] & 0xf8)) return false;
  Feedback result;
  for (size_t i = 0; i < kArmJoints; ++i)
  {
    const int raw = readSigned(frame + 2 + 2*i);
    if (raw < -18000 || raw > 18000) return false;
    result.position[i] = raw * kAngleUnit;
    result.velocity[i] = readSigned(frame + 14 + 2*i) * kAngleUnit;
  }
  result.grip = frame[26]; result.grip_state = frame[27]; result.mode = frame[28];
  result.position[6] = kGripperOpen * (1 - result.grip / 255.);
  // 协议没有夹爪速度，速度接口填 0；不能当作实测速度。
  feedback = result;
  return true;
}
std::vector<Feedback> FeedbackParser::append(const uint8_t *data, size_t size)
{
  std::vector<Feedback> frames;
  for (size_t i = 0; i < size; ++i)
  {
    buffer_.push_back(data[i]);
    while (buffer_.size() >= 2)
    {
      if (buffer_[0] != 0x55 || buffer_[1] != 0xaa)
      {
        buffer_.erase(buffer_.begin());
        continue;
      }
      if (buffer_.size() < kRxSize) break;
      Feedback f;
      if (decodeFeedback(buffer_.data(), f))
      {
        frames.push_back(f);
        buffer_.erase(buffer_.begin(), buffer_.begin() + kRxSize);
      }
      else
      {
        ++rejected_;
        buffer_.erase(buffer_.begin()); // 向后移动一个字节重新搜索，保留重叠帧头。
      }
    }
  }
  return frames;
}
void FeedbackParser::clear() { buffer_.clear(); rejected_ = 0; }
} // namespace serial_driver
