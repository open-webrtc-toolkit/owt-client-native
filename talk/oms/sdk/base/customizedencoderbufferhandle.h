// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H
#define OMS_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H
#include "rtc_base/atomicops.h"
#include "rtc_base/refcount.h"
#include "talk/oms/sdk/base/nativehandlebuffer.h"
#include "talk/oms/sdk/include/cpp/oms/base/videoencoderinterface.h"
namespace oms {
namespace base {
// This structure is to be included in the native handle
// that is passed to customized encoder proxy.
class CustomizedEncoderBufferHandle{
 public:
  VideoEncoderInterface* encoder;
  size_t width;
  size_t height;
  uint32_t fps;
  uint32_t bitrate_kbps;
  virtual ~CustomizedEncoderBufferHandle() {}
};
class EncodedFrameBuffer : public VideoFrameBuffer {
 public:
  EncodedFrameBuffer(CustomizedEncoderBufferHandle* native_handle)
      : native_handle_(native_handle) {
    if (native_handle) {
      width_ = native_handle->width;
      height_ = native_handle->height;
    }
  }
  virtual ~EncodedFrameBuffer() {
    if (native_handle_) {
      delete native_handle_;
      native_handle_ = nullptr;
    }
  }
  Type type() const override { return Type::kNative; }
  int width() const override { return width_; }
  int height() const override { return height_; }
  rtc::scoped_refptr<I420BufferInterface> ToI420() override {
    RTC_NOTREACHED();
    return nullptr;
  }
  void* native_handle() { return native_handle_; }
 private:
  CustomizedEncoderBufferHandle* native_handle_;
  size_t width_;
  size_t height_;
};
}  // namespace base
}  // namespace oms
#endif