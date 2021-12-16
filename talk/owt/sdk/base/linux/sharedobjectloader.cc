// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <dlfcn.h>

namespace owt {
namespace base {

SharedObjectLoader::SharedObjectLoader(const char* path)
    : shared_object(dlopen(path, RTLD_NOW), dlclose) {}

SharedObjectLoader::~SharedObjectLoader() {}

bool SharedObjectLoader::IsLoaded() const {
  return shared_object_.get();
}

void* SharedObjectLoader::get_symbol(const char* name) const {
  return shared_object_ ? dlsym(shared_object.get(), name) : nullptr;
}

}  // namespace base
}  // namespace owt
