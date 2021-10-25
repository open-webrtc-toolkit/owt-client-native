// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_SHAREDOBJECTLOADER_H_
#define OWT_BASE_SHAREDOBJECTLOADER_H_

namespace owt {
namespace base {

class SharedObjectLoader {
    public:
  explicit SharedObjectLoader(const char* name);
  ~SharedObjectLoader();

  bool IsLoaded() const;

  void* GetSymbol(const char* name);

 private:
  void* shared_object_;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_SHAREDOBJECTLOADER_H_
