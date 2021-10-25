// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <Windows.h>

namespace owt {
namespace base {

SharedObjectLoader::SharedObjectLoader(const char* name)
    : shared_object_(LoadLibraryA(name)) {}

SharedObjectLoader::~SharedObjectLoader() {
  if (shared_object_) {
    FreeLibrary(reinterpret_cast<HMODULE>(shared_object_));
  }
}

bool SharedObjectLoader::IsLoaded() const {
  return shared_object_;
}

void* SharedObjectLoader::GetSymbol(const char* name) {
  return shared_object_
             ? GetProcAddress(reinterpret_cast<HMODULE>(shared_object_), name)
             : nullptr;
}

}  // namespace base
}  // namespace owt
