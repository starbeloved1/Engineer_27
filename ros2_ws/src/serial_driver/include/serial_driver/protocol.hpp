#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace serial_driver
{
constexpr size_t kArmJoints = 6;
constexpr size_t kJoints = 7;
constexpr size_t kTxSize = 17;
constexpr size_t kRxSize = 31;
constexpr double kPi = 3.14159265358979323846;
constexpr double kAngleUnit = kPi / 18000.0;
constexpr double kGripperOpen = 0.01;
using JointValues = std::array<double, kJoints>;
using TxFrame = std::array<uint8_t, kTxSize>;
struct Feedback
{
  JointValues position{}, velocity{};
  uint8_t grip = 0, grip_state = 0, mode = 0;
};
uint8_t checksum(const uint8_t *data, size_t size);
TxFrame encodeCommand(const JointValues &position);
// 严格校验帧头、帧尾、累加和及位置范围；未知 mode 仍解码，交给控制权逻辑处理。
bool decodeFeedback(const uint8_t *frame, Feedback &feedback);
class FeedbackParser
{
public:
  std::vector<Feedback> append(const uint8_t *data, size_t size);
  void clear();
  uint64_t rejected() const { return rejected_; }
private:
  std::vector<uint8_t> buffer_;
  uint64_t rejected_ = 0;
};
} // namespace serial_driver
