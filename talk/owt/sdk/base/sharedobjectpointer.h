// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_SHAREDOBJECTPOINTER_H_
#define OWT_BASE_SHAREDOBJECTPOINTER_H_

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <memory>
#include <string>

namespace owt {
namespace base {

// This class is derived from openvino inference engine
template <typename T>
struct SOTrait {};

// This class is derived from openvino inference engine
template <typename T>
class SharedObjectPointer {
 public:
  explicit SharedObjectPointer(const char* path) : so_(path) { Load(); }

  explicit SharedObjectPointer(const SharedObjectLoader& loader) : so(loader) {
    Load();
  }

  explicit SharedObjectPointer(SharedObjectLoader&& loader)
      : so(std::move(loader)) {
    Load();
  }

  bool IsLoaded() const { return so_.IsLoaded() && ptr_.get(); }

  T* operator->() { return ptr_.get(); }

 protected:
  void Load() {
    if (so_.IsLoaded()) {
      using Creator = T*();
      using Destroyer = void(T*);
      Creator* creator = reinterpret_cast<Creator*>(
          so_.GetSymbol(std::string("Create") + SOTrait<T>::name));
      Destroyer* destroyer = reinterpret_cast<Destroyer*>(
          so_.GetSymbol(std::string("Destroy") + SOTrait<T>::name));
      if (creator && destroyer) {
        ptr_ = std::shared_ptr<T>(creator(), destroyer);
      }
    }
  }

  SharedObjectLoader so_;
  std::shared_ptr<T> ptr_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_SHAREDOBJECTPOINTER_H_
