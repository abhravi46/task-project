#include "framework.hpp"

Server::Server(Interface *interface) : interface_(interface) {
  if (interface_ == nullptr) {
    return;
  }
  interface_->Init();
}

Server::~Server() { Close(); }

bool Server::Open() {
  if (opened)
    return true;
  if (interface_ == nullptr)
    return -1;
  // Update the framework handle with the fd from the interface
  handle_ = interface_->Open();
  opened = true;
  return (handle_ > 0) ? true : false;
}

void Server::Close() {
  if (interface_ != nullptr) {
    interface_->Disconnect(con_fd_);
    interface_->Close(con_fd_);
    interface_->Close(handle_);
  }
  // Reset the framework handle when closing
  handle_ = -1;
  con_fd_ = -1;
  opened = false;
}

bool Server::Listen() {
  if (interface_ == nullptr) {
    return false;
  }
  con_fd_ = interface_->Listen(handle_);
  return (con_fd_ > 0) ? true : false;
}

bool Server::Connect() {
  /* This function does nothing on the server*/
  return true;
}

bool Server::Disconnect() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Disconnect(con_fd_);
}

ssize_t Server::Write(const void *buffer, size_t size) {
  if (interface_ == nullptr) {
    return -1;
  }
  return interface_->Write(con_fd_, buffer, size);
}

ssize_t Server::Read(void *buffer, size_t size) {
  if (interface_ == nullptr) {
    return -1;
  }
  return interface_->Read(con_fd_, buffer, size);
}
