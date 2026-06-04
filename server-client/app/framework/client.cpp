#include "framework.hpp"

Client::Client(Interface *interface) : interface_(interface) {
  if (interface_ == nullptr) {
    return;
  }
  interface_->Init();
}

Client::~Client() { Close(); }

bool Client::Open() {
  if (opened)
    return true;
  if (interface_ == nullptr)
    return false;

  // Update the framework handle with the fd from the interface
  handle_ = interface_->Open();
  opened = true;
  return (handle_ > 0) ? true : false;
}

void Client::Close() {
  if (interface_ != nullptr) {
    interface_->Disconnect(handle_);
    interface_->Close(handle_);
  }
  opened = false;
}

bool Client::Listen() {
  /* This function does nothing on the client */
  return true;
}

bool Client::Connect() {
  if (interface_ == nullptr || !opened) {
    return false;
  }
  return interface_->Connect(handle_);
}

bool Client::Disconnect() {
  if (interface_ == nullptr || !opened) {
    return false;
  }
  return interface_->Disconnect(handle_);
}

ssize_t Client::Write(const void *buffer, size_t size) {
  if (interface_ == nullptr || !opened) {
    return -1;
  }
  return interface_->Write(handle_, buffer, size);
}

ssize_t Client::Read(void *buffer, size_t size) {
  if (interface_ == nullptr || !opened) {
    return -1;
  }
  return interface_->Read(handle_, buffer, size);
}
