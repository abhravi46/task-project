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

// The socket fd is owned by the caller (it is returned from Open()), so there
// is nothing to release here; the owner is responsible for closing it via
// Close(fd).
LocalLoopback::~LocalLoopback() {}

bool LocalLoopback::Init() {
  std::memset(buffer_, 0, kBufferSize);
  write_pos_ = 0;
  read_pos_ = 0;
  data_size_ = 0;
  initialized_ = true;
  is_open_ = false;
  is_connected_ = false;
  return true;
}

int LocalLoopback::Open() {
  if (!initialized_) {
    return -1;
  }
  // Create a socket for loopback (network-specific)
  int fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    return -1;
  }
  is_open_ = true;
  return fd_;
}

void LocalLoopback::Close(int fd_) {
  if (fd_ >= 0) {
    close(fd_);
  }
  is_open_ = false;
  write_pos_ = 0;
  read_pos_ = 0;
  data_size_ = 0;
  std::memset(buffer_, 0, kBufferSize);
}

int LocalLoopback::Listen(int fd_) {
  // Listen on the socket provided by fd_ (network-specific)
  if (fd_ < 0) {
    return -1;
  }

  // Allow the address to be reused so the port can be rebound after restart
  int opt = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    return -1;
  }

  // Bind the socket to the loopback address and configured port
  struct sockaddr_in serv_addr;
  std::memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_);

  // Convert IPv4 address from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    return -1;
  }

  if (bind(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    return -1;
  }

  // Put the socket into the listening state for incoming connections
  if (listen(fd_, SOMAXCONN) < 0) {
    return -1;
  }

  // Accept an incoming connection (blocking call)
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  std::memset(&client_addr, 0, sizeof(client_addr));

  int client_fd = accept(fd_, (struct sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    return -1;
  }

  // The accepted connection is now established; mark the server connected so
  // Write() can send data back (mirrors Connect() on the client side).
  is_connected_ = true;

  // Return the client file descriptor for the accepted connection
  return client_fd;
}

bool LocalLoopback::Connect(int fd_) {
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
  is_connected_ = true;
  return is_open_;
}

bool LocalLoopback::Disconnect(int fd_) {
  // Disconnect the loopback interface
  if (is_connected_ && fd_ >= 0) {
    // Shutdown the socket connection
    shutdown(fd_, SHUT_RDWR);
  }
  is_open_ = false;
  return true;
}

ssize_t LocalLoopback::Write(int fd_, const void *buffer, size_t size) {
  if (!is_open_ || !is_connected_ || buffer == nullptr || size == 0) {
    return 0;
  }
  // Try network write first (network-specific)
  if (fd_ >= 0) {
    ssize_t result;
    int retries = 0;
    size_t total = size;
    size_t prev_size = size;
    do {
      result = write(fd_, buffer, size);
      size -= (result > 0) ? result : 0;
      if (prev_size == size)
        retries++;
      else
        retries = 0;
      prev_size = size;
    } while (size && retries < 3 && (result > 0));
    if (result > 0 && (retries < 3)) {
      return result;
    }
    if (retries >= 3) {
      return total - size;
    }
  }
  return 0;
}

ssize_t LocalLoopback::Read(int fd_, void *buffer, size_t size) {
  if (!is_open_ || !is_connected_ || buffer == nullptr || size == 0) {
    return 0;
  }

  // Try network read first (network-specific)
  if (fd_ >= 0) {
    ssize_t result = read(fd_, buffer, size);
    return result;
  }
  return 0;
}
