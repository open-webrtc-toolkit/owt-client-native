// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <Windows.h>

#include "third_party/webrtc/rtc_base/logging.h"

namespace owt {
namespace base {

SharedObjectLoader::SharedObjectLoader(const char* path)
    : shared_object_(LoadLibraryA(path), FreeLibrary) {
  if (!shared_object_) {
    RTC_LOG(LS_WARNING) << "Shared object " << path << " is not loaded.";
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
