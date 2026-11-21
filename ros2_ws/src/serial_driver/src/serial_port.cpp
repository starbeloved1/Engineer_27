#include "serial_driver/serial_port.hpp"
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <sys/file.h>
#include <termios.h>
#include <unistd.h>
#include <utility>

namespace serial_driver
{
namespace
{
void fail(const std::string &what) { throw std::runtime_error(what + ": " + std::strerror(errno)); }
speed_t baudValue(int baud)
{
  switch (baud)
  {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 921600: return B921600;
    default: throw std::invalid_argument("不支持的串口波特率: " + std::to_string(baud));
  }
}
}
SerialPort::~SerialPort() { close(); }
void SerialPort::open(const std::string &device, int baud, bool flush_output)
{
  close();
  const auto speed = baudValue(baud);
  fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
  if (fd_ < 0) fail("打开 " + device);
  try
  {
    if (flock(fd_, LOCK_EX | LOCK_NB) != 0) fail("串口已被其他驱动占用");
    termios tty{};
    if (tcgetattr(fd_, &tty) != 0) fail("tcgetattr");
    cfmakeraw(&tty);
    tty.c_cflag = (tty.c_cflag & ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS)) | CS8 | CLOCAL | CREAD;
    tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 0;
    if (cfsetispeed(&tty, speed) || cfsetospeed(&tty, speed) || tcsetattr(fd_, TCSANOW, &tty))
      fail("配置串口 8N1");
    if (tcflush(fd_, flush_output ? TCIOFLUSH : TCIFLUSH)) fail("清理串口缓冲");
  }
  catch (...) { close(); throw; }
}
void SerialPort::swap(SerialPort &other) noexcept { std::swap(fd_, other.fd_); }
void SerialPort::close()
{
  if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
}
std::vector<uint8_t> SerialPort::readAvailable(int timeout_ms)
{
  pollfd p{fd_, POLLIN, 0};
  const int ready = ::poll(&p, 1, timeout_ms);
  if (ready < 0 && errno != EINTR) fail("串口 poll");
  if (p.revents & (POLLHUP | POLLERR | POLLNVAL))
    throw std::runtime_error("串口断开或发生 I/O 错误");
  if (ready <= 0 || !(p.revents & POLLIN)) return {};
  std::vector<uint8_t> result;
  uint8_t buffer[256];
  for (int count = 0; count < 16; ++count)
  {
    const auto n = ::read(fd_, buffer, sizeof(buffer));
    if (n > 0) result.insert(result.end(), buffer, buffer + n);
    else if (n == 0 || errno == EAGAIN || errno == EWOULDBLOCK) break;
    else if (errno != EINTR) fail("串口读取");
  }
  return result;
}
void SerialPort::writeFrame(const uint8_t *data, size_t size)
{
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
  size_t offset = 0;
  while (offset < size)
  {
    const auto n = ::write(fd_, data + offset, size - offset);
    if (n > 0) offset += static_cast<size_t>(n);
    else if (n < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) fail("串口发送");
    if (offset == size) break;
    if (std::chrono::steady_clock::now() >= deadline) throw std::runtime_error("串口整帧发送超时");
    pollfd p{fd_, POLLOUT, 0};
    if (::poll(&p, 1, 2) < 0 && errno != EINTR) fail("串口发送 poll");
    if (p.revents & (POLLHUP | POLLERR | POLLNVAL)) throw std::runtime_error("发送期间串口断开");
  }
}
void SerialPort::discardOutput() { if (fd_ >= 0 && tcflush(fd_, TCOFLUSH)) fail("清理待发目标"); }
} // namespace serial_driver
