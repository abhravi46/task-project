#include "framework.hpp"

OpenClient::OpenClient(Interface *interface)
    : interface_(interface) {}

OpenClient::~OpenClient() { Close(); }

bool OpenClient::Open() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Open();
}

void OpenClient::Close() {
  if (interface_ != nullptr) {
    interface_->Close();
  }
}

bool OpenClient::Listen() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Listen();
}

bool OpenClient::Connect() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Connect();
}

bool OpenClient::Disconnect() {
  if (interface_ == nullptr) {
    return false;
  }
  return interface_->Disconnect();
}

ssize_t OpenClient::Write(const void *buffer, size_t size) {
  if (interface_ == nullptr) {
    return -1;
  }
  return interface_->Write(buffer, size);
}

ssize_t OpenClient::Read(void *buffer, size_t size) {
  if (interface_ == nullptr) {
    return -1;
  }
  return interface_->Read(buffer, size);
}
