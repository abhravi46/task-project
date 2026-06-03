#include "interface.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

LocalLoopback::LocalLoopback(int port)
    : Interface(port, "127.0.0.1"), write_pos_(0), read_pos_(0), data_size_(0),
      is_open_(false) {
  std::memset(buffer_, 0, kBufferSize);
}

LocalLoopback::~LocalLoopback() { Close(); }

bool LocalLoopback::Init() {
  std::memset(buffer_, 0, kBufferSize);
  write_pos_ = 0;
  read_pos_ = 0;
  data_size_ = 0;
  initialized_ = true;
  return true;
}

bool LocalLoopback::Open() {
  if (!initialized_) {
    return false;
  }
  // Create a socket for loopback (network-specific)
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    return false;
  }
  is_open_ = true;
  return true;
}

void LocalLoopback::Close() {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
  is_open_ = false;
  write_pos_ = 0;
  read_pos_ = 0;
  data_size_ = 0;
  std::memset(buffer_, 0, kBufferSize);
}

bool LocalLoopback::Listen() {
  // Listen is not applicable for loopback
  return false;
}

bool LocalLoopback::Connect() {
  // For loopback, connect to localhost (network-specific)
  if (fd_ < 0) {
    return false;
  }

  struct sockaddr_in serv_addr;
  std::memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_);

  // Convert IPv4 address from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    return false;
  }

  if (connect(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    return false;
  }

  return is_open_;
}

bool LocalLoopback::Disconnect() {
  // Disconnect the loopback interface
  if (fd_ >= 0) {
    // Shutdown the socket connection
    shutdown(fd_, SHUT_RDWR);
  }
  is_open_ = false;
  return true;
}

ssize_t LocalLoopback::Write(const void *buffer, size_t size) {
  if (!is_open_ || buffer == nullptr || size == 0) {
    return -1;
  }

  // Try network write first (network-specific)
  if (fd_ >= 0) {
    ssize_t result = write(fd_, buffer, size);
    if (result > 0) {
      return result;
    }
  }

  // Fall back to buffer loopback
  size_t available = kBufferSize - data_size_;
  if (available == 0) {
    return 0; // Buffer full
  }

  size_t to_write = std::min(size, available);
  const char *src = static_cast<const char *>(buffer);

  // Write data, handling wrap-around
  for (size_t i = 0; i < to_write; ++i) {
    buffer_[write_pos_] = src[i];
    write_pos_ = (write_pos_ + 1) % kBufferSize;
  }

  data_size_ += to_write;
  return static_cast<ssize_t>(to_write);
}

ssize_t LocalLoopback::Read(void *buffer, size_t size) {
  if (!is_open_ || buffer == nullptr || size == 0) {
    return -1;
  }

  // Try network read first (network-specific)
  if (fd_ >= 0) {
    ssize_t result = read(fd_, buffer, size);
    if (result > 0) {
      return result;
    }
  }

  // Fall back to buffer loopback
  if (data_size_ == 0) {
    return 0; // No data available
  }

  size_t to_read = std::min(size, data_size_);
  char *dst = static_cast<char *>(buffer);

  // Read data, handling wrap-around
  for (size_t i = 0; i < to_read; ++i) {
    dst[i] = buffer_[read_pos_];
    read_pos_ = (read_pos_ + 1) % kBufferSize;
  }

  data_size_ -= to_read;
  return static_cast<ssize_t>(to_read);
}
