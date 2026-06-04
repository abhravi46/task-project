#pragma once

#include <cstddef>
#include <sys/types.h>

#include "interface.hpp"

/**
 * @brief Base framework class providing interface for network communication
 */
class FrmwrkBase {
public:
  FrmwrkBase() : handle_(-1) {}
  virtual ~FrmwrkBase() = default;
  virtual bool Open() = 0;
  virtual void Close() = 0;
  virtual bool Listen() = 0;
  virtual bool Connect() = 0;
  virtual bool Disconnect() = 0;
  virtual ssize_t Write(const void *buffer, size_t size) = 0;
  virtual ssize_t Read(void *buffer, size_t size) = 0;

  /**
   * @brief Get the socket handle (file descriptor)
   * @return The socket file descriptor, or -1 if not set
   */
  int GetHandle() const { return handle_; }

protected:
  // Socket handle (file descriptor) that gets updated with the fd for the
  // socket
  int handle_;
  int con_fd_;
  bool opened = false;

private:
  FrmwrkBase(const FrmwrkBase &) = delete;
  FrmwrkBase &operator=(const FrmwrkBase &) = delete;
};

/**
 * @brief Server implementation of the framework base class
 *
 * This class provides server-side functionality for accepting
 * incoming connections and handling client requests.
 */
class Server : public FrmwrkBase {
public:
  /**
   * @brief Constructor
   * @param interface Pointer to the network interface implementation
   */
  explicit Server(Interface *interface);

  /**
   * @brief Destructor
   */
  ~Server() override;

  /**
   * @brief Open a server socket for listening
   * @return File descriptor on success, -1 on failure
   */
  bool Open() override;

  /**
   * @brief Close the server socket
   */
  void Close() override;

  /**
   * @brief Listen for incoming client connections
   * @return true on success, false on failure
   */
  bool Listen() override;

  /**
   * @brief Connect is not applicable for server (delegates to interface)
   * @return result from interface
   */
  bool Connect() override;

  /**
   * @brief Disconnect is not applicable for server (delegates to interface)
   * @return result from interface
   */
  bool Disconnect() override;

  /**
   * @brief Write data to the connected client
   * @param buffer Pointer to the data buffer
   * @param size Number of bytes to write
   * @return Number of bytes written, or -1 on error
   */
  ssize_t Write(const void *buffer, size_t size) override;

  /**
   * @brief Read data from the connected client
   * @param buffer Pointer to the buffer to store read data
   * @param size Maximum number of bytes to read
   * @return Number of bytes read, or -1 on error
   */
  ssize_t Read(void *buffer, size_t size) override;

private:
  // Interface for network operations
  Interface *interface_;
};

/**
 * @brief Client implementation of the framework base class
 *
 * This class provides client-side functionality for connecting
 * to a server and exchanging data.
 */
class Client : public FrmwrkBase {
public:
  /**
   * @brief Constructor
   * @param interface Pointer to the network interface implementation
   */
  explicit Client(Interface *interface);

  /**
   * @brief Destructor
   */
  ~Client() override;

  /**
   * @brief Open a client socket
   * @return File descriptor on success, -1 on failure
   */
  bool Open() override;

  /**
   * @brief Close the client socket
   */
  void Close() override;

  /**
   * @brief Listen is not applicable for client (delegates to interface)
   * @return result from interface
   */
  bool Listen() override;

  /**
   * @brief Connect to a remote server
   * @return true on success, false on failure
   */
  bool Connect() override;

  /**
   * @brief Disconnect from the server
   * @return true on success, false on failure
   */
  bool Disconnect() override;

  /**
   * @brief Write data to the server
   * @param buffer Pointer to the data buffer
   * @param size Number of bytes to write
   * @return Number of bytes written, or -1 on error
   */
  ssize_t Write(const void *buffer, size_t size) override;

  /**
   * @brief Read data from the server
   * @param buffer Pointer to the buffer to store read data
   * @param size Maximum number of bytes to read
   * @return Number of bytes read, or -1 on error
   */
  ssize_t Read(void *buffer, size_t size) override;

protected:
  // Interface for network operations
  Interface *interface_;
};
