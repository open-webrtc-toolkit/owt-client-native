// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <Windows.h>
#include <string>

#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace base {
namespace {

std::string GetErrorAsString(DWORD message_id) {
  LPSTR buffer = NULL;
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
      message_id, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&buffer,
      0, NULL);
  std::string message(buffer, size);
  LocalFree(buffer);
  return message;
}

}  // namespace

SharedObjectLoader::SharedObjectLoader(const char* path)
    : shared_object_(LoadLibraryA(path), FreeLibrary) {
  if (!shared_object_) {
    DWORD message_id = GetLastError();
    RTC_LOG(LS_WARNING) << "Load shared library " << path << " failed. "
                        << "Error code " << message_id << ": "
                        << GetErrorAsString(message_id);
  }
}

SharedObjectLoader::~SharedObjectLoader() {}

bool SharedObjectLoader::IsLoaded() const {
  return shared_object_.get();
}

void* SharedObjectLoader::GetSymbol(const char* name) const {
  if (shared_object_) {
    return GetProcAddress(reinterpret_cast<HMODULE>(shared_object_.get()),
                          name);
  }
  return nullptr;
}

void* SharedObjectLoader::GetSymbol(const std::string& name) const {
  return GetSymbol(name.c_str());
}

}  // namespace base
}  // namespace owt
