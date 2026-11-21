#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace serial_driver
{
// Linux termios：8N1、无流控、独占、非阻塞读取；不依赖额外串口库。
class SerialPort
{
public:
  SerialPort() = default;
  ~SerialPort();
  SerialPort(const SerialPort &) = delete;
  SerialPort &operator=(const SerialPort &) = delete;
  void open(const std::string &device, int baud, bool flush_output = true);
  void swap(SerialPort &other) noexcept;
  void close();
  std::vector<uint8_t> readAvailable(int timeout_ms = 0);
  void writeFrame(const uint8_t *data, size_t size);
  void discardOutput();
private:
  int fd_ = -1;
};
} // namespace serial_driver
