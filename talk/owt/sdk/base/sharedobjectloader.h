// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_SHAREDOBJECTLOADER_H_
#define OWT_BASE_SHAREDOBJECTLOADER_H_

#include <memory>
#include <string>

namespace owt {
namespace base {

// This class is derived from openvino inference engine
class SharedObjectLoader {
 public:
  explicit SharedObjectLoader(const char* path);
  ~SharedObjectLoader();

  bool IsLoaded() const;

  void* GetSymbol(const char* name) const;

  void* GetSymbol(const std::string& name) const;

 private:
  std::shared_ptr<void> shared_object_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_SHAREDOBJECTLOADER_H_
