// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_LINUX_XWINDOWNATIVEFRAME_H_
#define OWT_BASE_LINUX_XWINDOWNATIVEFRAME_H_

#include <va/va.h>

namespace owt {
namespace base {

typedef void (*PFN_RETURN_BUFFER)(void *data, uint32_t bufid);
struct NativeXWindowSurfaceHandle {
  VADisplay display_;
  VASurfaceID surface_;
  int width_;
  int height_;
  int frameno;
  uint32_t bufferid;
  void *data;
  void *pfnReturnBuffer;

};

}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_LINUX_XWINDOWNATIVEFRAME_H_
