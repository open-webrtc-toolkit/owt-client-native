// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include "libyuv/convert.h"
#include "webrtc/rtc_base/byte_buffer.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/memory/aligned_malloc.h"
#include "webrtc/rtc_base/synchronization/mutex.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/rtc_base/time_utils.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "talk/owt/sdk/base/desktopcapturer.h"

using namespace rtc;
namespace owt {
namespace base {
///////////////////////////////////////////////////////////////////////
// Definition of private class BasicScreenCaptureThread that periodically
// generates frames.
///////////////////////////////////////////////////////////////////////
class BasicScreenCapturer::BasicScreenCaptureThread
    : public rtc::Thread,
      public rtc::MessageHandler {
 public:
  explicit BasicScreenCaptureThread(BasicScreenCapturer* capturer)
      : rtc::Thread(rtc::SocketServer::CreateDefault()),
        capturer_(capturer),
        finished_(false) {
    waiting_time_ms_ = 1000 / 30;  // For basic capturer, fix it to 30fps
  }

  virtual ~BasicScreenCaptureThread() { Stop(); }
  // Override virtual method of parent Thread. Context: Worker Thread.
  virtual void Run() {
    // Read the first frame and start the message pump. The pump runs until
    // Stop() is called externally or Quit() is called by OnMessage().
    if (capturer_) {
      capturer_->CaptureFrame();
      rtc::Thread::Current()->Post(RTC_FROM_HERE, this);
      rtc::Thread::Current()->ProcessMessages(kForever);
    }
    webrtc::MutexLock lock(&mutex_);
    finished_ = true;
  }
  // Override virtual method of parent MessageHandler. Context: Worker Thread.
  virtual void OnMessage(rtc::Message* /*pmsg*/) {
    if (capturer_) {
      capturer_->CaptureFrame();
      rtc::Thread::Current()->PostDelayed(RTC_FROM_HERE, waiting_time_ms_,
                                          this);
    } else {
      rtc::Thread::Current()->Quit();
    }
  }
  // Check if Run() is finished.
  bool Finished() const {
    webrtc::MutexLock lock(&mutex_);
    return finished_;
  }
 private:
  BasicScreenCapturer* capturer_;
  mutable webrtc::Mutex mutex_;
  bool finished_;
  int waiting_time_ms_;
  RTC_DISALLOW_COPY_AND_ASSIGN(BasicScreenCaptureThread);
};
/////////////////////////////////////////////////////////////////////
// Implementation of class BasicScreenCapturer.
/////////////////////////////////////////////////////////////////////
BasicScreenCapturer::BasicScreenCapturer(webrtc::DesktopCaptureOptions options)
    : screen_capture_thread_(nullptr),
      width_(0),
      height_(0),
      frame_buffer_capacity_(0),
      frame_buffer_(nullptr),
      screen_capture_options_(options) {
  screen_capturer_ =
      webrtc::DesktopCapturer::CreateScreenCapturer(screen_capture_options_);
}
BasicScreenCapturer::~BasicScreenCapturer() {
  DeRegisterCaptureDataCallback();
  if (capture_started_)
    StopCapture();
}

int32_t BasicScreenCapturer::StartCapture(
    const webrtc::VideoCaptureCapability& capabilit) {
  if (capture_started_) {
    RTC_LOG(LS_ERROR) << "Basic Screen Capturerer is already running";
    return 0;
  }
  if (!screen_capturer_.get()) {
    RTC_LOG(LS_ERROR) << "Desktop capturer creation failed, not able to start it";
    return -1;
  }
  screen_capturer_->Start(this);

  screen_capture_thread_.reset(new BasicScreenCaptureThread(this));
  bool ret = screen_capture_thread_->Start();
  if (!ret) {
    RTC_LOG(LS_ERROR) << "Screen capture thread failed to start";
    return -1;
  }

  capture_started_ = true;
  return 0;
}

bool BasicScreenCapturer::IsRunning() {
  return screen_capture_thread_ && !screen_capture_thread_->Finished();
}

int32_t BasicScreenCapturer::StopCapture() {
  if (!capture_started_)
    return 0;
  if (screen_capture_thread_) {
    screen_capture_thread_->Quit();
    screen_capture_thread_.reset();
  }
  capture_started_ = false;
  return 0;
}

bool BasicScreenCapturer::CaptureStarted() {
  return capture_started_;
}

int32_t BasicScreenCapturer::CaptureSettings(
    webrtc::VideoCaptureCapability& settings) {
  settings.width = width_;
  settings.height = height_;
  settings.maxFPS = 30;  // We should not hardcode it.
  settings.videoType = webrtc::VideoType::kI420;

  return 0;
}

int32_t BasicScreenCapturer::SetCaptureRotation(
    webrtc::VideoRotation rotation) {
  // Not implemented.
  return 0;
}

int BasicScreenCapturer::I420DataSize(int height,
                                      int stride_y,
                                      int stride_u,
                                      int stride_v) {
  return stride_y * height + (stride_u + stride_v) * ((height + 1) / 2);
}
void BasicScreenCapturer::AdjustFrameBuffer(int32_t width, int32_t height) {
  if (width_ != width || height != height_ || !frame_buffer_) {
    RTC_LOG(LS_VERBOSE) << "Allocate new memory for frame buffer.";
    width_ = width;
    height_ = height;
    int stride_y = width_;
    int stride_uv = (width_ + 1) / 2;
    frame_buffer_ = webrtc::I420Buffer::Create(width_, height_, stride_y,
                                               stride_uv, stride_uv);
    frame_buffer_capacity_ =
        I420DataSize(height_, stride_y, stride_uv, stride_uv);
    return;
  }
}
// Executed in the context of BasicScreenCaptureThread.
void BasicScreenCapturer::CaptureFrame() {
  // invoke underlying desktop capture to capture one frame.
  if (!screen_capturer_.get()) {
    RTC_LOG(LS_ERROR) << "Failed to capture one screen frame";
    return;
  }
  return screen_capturer_->CaptureFrame();
}
void BasicScreenCapturer::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
    RTC_LOG(LS_ERROR) << "Failed to cpature one screen frame.";
    return;
  }
  if (!data_callback_)
    return;
  int32_t frame_width = frame->size().width();
  int32_t frame_height = frame->size().height();
  uint8_t* frame_data_rgba = frame->data();
  int frame_stride = frame->stride();
  if (frame_width == 0 || frame_height == 0 || frame_data_rgba == nullptr) {
    RTC_LOG(LS_ERROR) << "Invalid screen data";
    return;
  }
  // The captured frame is of memory layout ABRG. convert it to I420 as
  // required.
  AdjustFrameBuffer(frame_width, frame_height);
  libyuv::ARGBToI420(frame_data_rgba, frame_stride,
                     frame_buffer_->MutableDataY(), frame_buffer_->StrideY(),
                     frame_buffer_->MutableDataU(), frame_buffer_->StrideU(),
                     frame_buffer_->MutableDataV(), frame_buffer_->StrideV(),
                     frame_width, frame_height);
  webrtc::VideoFrame captured_frame =
      webrtc::VideoFrame::Builder()
          .set_video_frame_buffer(frame_buffer_)
          .set_timestamp_rtp(0)
          .set_timestamp_ms(rtc::TimeMillis())
          .set_rotation(webrtc::kVideoRotation_0)
          .build();

  captured_frame.set_ntp_time_ms(0);
  data_callback_->OnFrame(captured_frame);
}
}  // namespace base
}  // namespace owt
