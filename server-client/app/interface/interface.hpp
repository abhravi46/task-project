#pragma once

#include <cstddef>
#include <string>
#include <sys/types.h>

/**
 * @brief Interface class providing network communication abstraction
 *
 * This class defines the interface for network operations including
 * initialization, connection management, and data transfer.
 */
class Interface {
public:
  /**
   * @brief Constructor
   */
  Interface(int port, const std::string &address)
      : initialized_(false), port_(port), address_(address) {}

  /**
   * @brief Destructor
   */
  virtual ~Interface() = default;

  /**
   * @brief Initialize the interface
   * @return true on success, false on failure
   */
  virtual bool Init() = 0;

  /**
   * @brief Open a connection or resource
   * @return File descriptor on success, -1 on failure
   */
  virtual int Open() = 0;

  /**
   * @brief Close the connection or resource
   */
  virtual void Close(int fd_) = 0;

  /**
   * @brief Listen for incoming connections
   * @return true on success, false on failure
   */
  virtual int Listen(int fd_) = 0;

  /**
   * @brief Connect to a remote endpoint
   * @return true on success, false on failure
   */
  virtual bool Connect(int fd_) = 0;

  /**
   * @brief Disconnect from the remote endpoint
   * @return true on success, false on failure
   */
  virtual bool Disconnect(int fd_) = 0;

  /**
   * @brief Write data to the connection
   * @param buffer Pointer to the data buffer
   * @param size Number of bytes to write
   * @return Number of bytes written, or -1 on error
   */
  virtual ssize_t Write(int fd_, const void *buffer, size_t size) = 0;

  /**
   * @brief Read data from the connection
   * @param buffer Pointer to the buffer to store read data
   * @param size Maximum number of bytes to read
   * @return Number of bytes read, or -1 on error
   */
  virtual ssize_t Read(int fd_, void *buffer, size_t size) = 0;

protected:
  // Protected members for derived classes
  bool initialized_;
  int port_;
  std::string address_;

private:
  // Disable copy and assignment
  Interface(const Interface &) = delete;
  Interface &operator=(const Interface &) = delete;
};

/**
 * @brief Local loopback implementation of the interface
 *
 * This class provides a loopback implementation where data written
 * is immediately available for reading, useful for testing without
 * actual network communication.
 */
class LocalLoopback : public Interface {
public:
  /**
   * @brief Constructor
   * @param port Port number
   */
  explicit LocalLoopback(int port);

  /**
   * @brief Destructor
   */
  ~LocalLoopback() override;

  /**
   * @brief Initialize the loopback interface
   * @return true on success, false on failure
   */
  bool Init() override;

  /**
   * @brief Open the loopback interface
   * @return File descriptor on success, -1 on failure
   */
  int Open() override;

  /**
   * @brief Close the loopback interface
   */
  void Close(int fd_) override;

  /**
   * @brief Listen is not applicable for loopback (returns false)
   * @return false always
   */
  int Listen(int fd_) override;

  /**
   * @brief Connect is not applicable for loopback (returns true if open)
   * @return true if interface is open, false otherwise
   */
  bool Connect(int fd_) override;

  /**
   * @brief Disconnect the loopback interface
   * @return true on success, false on failure
   */
  bool Disconnect(int fd_) override;

  /**
   * @brief Write data to the loopback buffer
   * @param buffer Pointer to the data buffer
   * @param size Number of bytes to write
   * @return Number of bytes written, or -1 on error
   */
  ssize_t Write(int fd_, const void *buffer, size_t size) override;

  /**
   * @brief Read data from the loopback buffer
   * @param buffer Pointer to the buffer to store read data
   * @param size Maximum number of bytes to read
   * @return Number of bytes read, or -1 on error
   */
  ssize_t Read(int fd_, void *buffer, size_t size) override;

private:
  // Loopback-specific members
  static constexpr size_t kBufferSize = 4096;
  char buffer_[kBufferSize];
  size_t write_pos_;
  size_t read_pos_;
  size_t data_size_;
  bool is_open_;
  bool is_connected_;
};
