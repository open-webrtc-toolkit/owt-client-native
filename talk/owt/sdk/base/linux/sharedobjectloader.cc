// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <dlfcn.h>

namespace owt {
namespace base {

SharedObjectLoader::SharedObjectLoader(const char* name)
    : shared_object(dlopen(name, RTLD_NOW)) {}

SharedObjectLoader::~SharedObjectLoader() {
  if (shared_object) {
    dlclose(shared_object);
  }
}

bool SharedObjectLoader::IsLoaded() const {
  return shared_object_;
}

void* SharedObjectLoader::get_symbol(const char* name) {
  return shared_object_ ? dlsym(shared_object, name) : nullptr;
}

}  // namespace base
}  // namespace owt
