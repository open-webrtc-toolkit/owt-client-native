// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H
#define OWT_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H
#include "rtc_base/ref_count.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/include/cpp/owt/base/videoencoderinterface.h"
namespace owt {
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

class CustomizedEncoderBufferHandle2 {
 public:
  CustomizedEncoderBufferHandle2()
      : encoder_event_callback_(nullptr),
        width_(0),
        height_(0),
        fps_(0),
        bitrate_kbps_(0),
        buffer_(nullptr),
        buffer_length_(0) {}

  virtual ~CustomizedEncoderBufferHandle2() {
    if (buffer_ != nullptr) {
      delete[] buffer_;
      buffer_ = nullptr;
    }
    encoder_event_callback_ = nullptr;
  }

 public:
  EncoderEventCallback* encoder_event_callback_;
  size_t width_;
  size_t height_;
  uint32_t fps_;
  uint32_t bitrate_kbps_;
  uint8_t* buffer_;
  size_t buffer_length_;
  EncodedImageMetaData meta_data_;
};

// Deprecated.
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
    RTC_DCHECK_NOTREACHED();
    return nullptr;
  }
  void* native_handle() { return native_handle_; }
 private:
  CustomizedEncoderBufferHandle* native_handle_;
  size_t width_ = 0;
  size_t height_ = 0;
};

class EncodedFrameBuffer2 : public VideoFrameBuffer {
 public:
  EncodedFrameBuffer2(CustomizedEncoderBufferHandle2* native_handle)
      : native_handle_(native_handle) {
    if (native_handle) {
      width_ = native_handle->width_;
      height_ = native_handle->height_;
    }
  }
  virtual ~EncodedFrameBuffer2() {
    if (native_handle_) {
      delete native_handle_;
      native_handle_ = nullptr;
    }
  }
  Type type() const override { return Type::kNative; }
  int width() const override { return width_; }
  int height() const override { return height_; }
  rtc::scoped_refptr<I420BufferInterface> ToI420() override {
    RTC_DCHECK_NOTREACHED();
    return nullptr;
  }
  void* native_handle() { return native_handle_; }

 private:
  CustomizedEncoderBufferHandle2* native_handle_;
  size_t width_;
  size_t height_;
};
}  // namespace base
}  // namespace owt
#endif