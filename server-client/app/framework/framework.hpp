#pragma once

#include <cstddef>
#include <sys/types.h>

#include "interface.hpp"

/**
 * @brief Base framework class providing interface for network communication
 */
class FrmwrkBase {
public:
  FrmwrkBase() = default;
  virtual ~FrmwrkBase() = default;
  virtual bool Open() = 0;
  virtual void Close() = 0;
  virtual bool Listen() = 0;
  virtual bool Connect() = 0;
  virtual bool Disconnect() = 0;
  virtual ssize_t Write(const void *buffer, size_t size) = 0;
  virtual ssize_t Read(void *buffer, size_t size) = 0;

protected:
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
class OpenServer : public FrmwrkBase {
public:
  /**
   * @brief Constructor
   * @param interface Pointer to the network interface implementation
   */
  explicit OpenServer(Interface* interface);

  /**
   * @brief Destructor
   */
  ~OpenServer() override;

  /**
   * @brief Open a server socket for listening
   * @return true on success, false on failure
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
  Interface* interface_;
};

/**
 * @brief Client implementation of the framework base class
 *
 * This class provides client-side functionality for connecting
 * to a server and exchanging data.
 */
class OpenClient : public FrmwrkBase {
public:
  /**
   * @brief Constructor
   * @param interface Pointer to the network interface implementation
   */
  explicit OpenClient(Interface* interface);

  /**
   * @brief Destructor
   */
  ~OpenClient() override;

  /**
   * @brief Open a client socket
   * @return true on success, false on failure
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

private:
  // Interface for network operations
  Interface* interface_;
};
