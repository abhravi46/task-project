#include "framework.hpp"

OpenServer::OpenServer(Interface *interface)
    : interface_(interface) {}

OpenServer::~OpenServer() { Close(); }

bool OpenServer::Open() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Open();
}

void OpenServer::Close() {
  if (interface_ != nullptr) {
    interface_->Close();
  }
}

bool OpenServer::Listen() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Listen();
}

bool OpenServer::Connect() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Connect();
}

bool OpenServer::Disconnect() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Disconnect();
}

ssize_t OpenServer::Write(const void *buffer, size_t size) {
  if (interface_ == nullptr) {
    return -1;
  }
  return interface_->Write(buffer, size);
}

ssize_t OpenServer::Read(void *buffer, size_t size) {
  if (interface_ == nullptr) {
    return -1;
  }
  return interface_->Read(buffer, size);
}
