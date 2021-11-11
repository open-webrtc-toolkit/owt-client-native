// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_SHAREDOBJECTPOINTER_H_
#define OWT_BASE_SHAREDOBJECTPOINTER_H_

#include "talk/owt/sdk/base/sharedobjectloader.h"

#include <memory>
#include <string>

#include "third_party/webrtc/rtc_base/logging.h"

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

  bool IsLoaded() const { return so_.IsLoaded(); }

  T* Get() { return ptr_.get(); }

 protected:
  void Load() {
    if (!so_.IsLoaded()) {
      return;
    }

    using Creator = T*();
    using Destroyer = void(T*);
    Creator* creator = reinterpret_cast<Creator*>(
        so_.GetSymbol(std::string("Create") + SOTrait<T>::name));
    Destroyer* destroyer = reinterpret_cast<Destroyer*>(
        so_.GetSymbol(std::string("Destroy") + SOTrait<T>::name));

    if (creator && destroyer) {
      ptr_ = std::shared_ptr<T>(creator(), destroyer);
    } else {
      RTC_LOG(LS_WARNING) << "Create" << SOTrait<T>::name  //
                       << " or Destroy" << SOTrait<T>::name
                       << " is missing in the shared object.";
    }
  }

  SharedObjectLoader so_;
  std::shared_ptr<T> ptr_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_SHAREDOBJECTPOINTER_H_
