// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <mutex>
#include "libyuv/convert.h"
#include "libyuv/scale_argb.h"
#include "talk/owt/sdk/base/desktopcapturer.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/byte_buffer.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/memory/aligned_malloc.h"
#include "webrtc/rtc_base/time_utils.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "webrtc/system_wrappers/include/sleep.h"
using namespace rtc;
namespace owt {
namespace base {
/////////////////////////////////////////////////////////////////////
// Implementation of class BasicWindowCapturer. Window capturer is
// different from window capturer as the window may be disposed
// at any time.
/////////////////////////////////////////////////////////////////////
void ScreenCaptureThread::Run() {
  ProcessMessages(kForever);
}
ScreenCaptureThread::~ScreenCaptureThread() {
  Stop();
}
BasicWindowCapturer::BasicWindowCapturer(
    webrtc::DesktopCaptureOptions options,
    std::unique_ptr<LocalScreenStreamObserver> observer)
    : width_(0),
      height_(0),
      frame_buffer_capacity_(0),
      frame_buffer_(nullptr),
      window_capture_options_(options),
      source_specified_(false),
      last_call_record_millis_(0),
      clock_(webrtc::Clock::GetRealTimeClock()),
      need_sleep_ms_(0),
      real_sleep_ms_(0),
      stopped_(false),
      observer_(std::move(observer)) {
  window_capturer_ =
      webrtc::DesktopCapturer::CreateWindowCapturer(window_capture_options_);
  worker_thread_.reset(new owt::base::ScreenCaptureThread());
  worker_thread_->SetName("CaptureInvokeThread", nullptr);
  worker_thread_->Start();

}

BasicWindowCapturer::~BasicWindowCapturer() {
  DeRegisterCaptureDataCallback();
  if (capture_started_)
    StopCapture();
}
void BasicWindowCapturer::WindowCaptureThreadFunc(void* pThis) {
  static_cast<BasicWindowCapturer*>(pThis)->CaptureThreadProcess();
}

bool BasicWindowCapturer::CaptureThreadProcess() {
  while (capture_started_) {
    uint64_t current_time = clock_->CurrentNtpInMilliseconds();
    lock_.Lock();
    if (last_call_record_millis_ == 0 ||
        (int64_t)(current_time - last_call_record_millis_) >= need_sleep_ms_) {
      if (source_specified_ && window_capturer_) {
        last_call_record_millis_ = current_time;
        window_capturer_->CaptureFrame();
      }
    }
    lock_.Unlock();
    int64_t cost_ms = clock_->CurrentNtpInMilliseconds() - current_time;
    need_sleep_ms_ = 33 - cost_ms;
    if (need_sleep_ms_ > 0) {
      current_time = clock_->CurrentNtpInMilliseconds();
      webrtc::SleepMs(need_sleep_ms_);
      real_sleep_ms_ = clock_->CurrentNtpInMilliseconds() - current_time;
    } else {
      RTC_LOG(LS_WARNING)
          << "Cost too much time on window frame capture. This may "
           "leads to large latency";
    }
  }
  return true;
}

void BasicWindowCapturer::InitOnWorkerThread() {
  if (!capture_thread_) {
    capture_thread_.reset(new rtc::PlatformThread(WindowCaptureThreadFunc, this,
                                                  "WindowCaptureThread", rtc::kHighPriority));
    capture_thread_->Start();
  }
}

int32_t BasicWindowCapturer::StartCapture(
    const webrtc::VideoCaptureCapability& capabilit) {
  if (capture_started_) {
    RTC_LOG(LS_INFO) << "Window Captureerer is already running.";
    return 0;
  }
  if (!window_capturer_.get()) {
    RTC_LOG(LS_ERROR)
        << "Desktop capturer creation failed, not able to start it";
    return -1;
  }

  capture_started_ = true;
  worker_thread_->Invoke<void>(
      RTC_FROM_HERE, rtc::Bind(&BasicWindowCapturer::InitOnWorkerThread, this));
  if (observer_) {
    std::unordered_map<int, std::string> window_map;
    int window_id;
    if (GetCurrentWindowList(&window_map)) {
      observer_->OnCaptureSourceNeeded(window_map, window_id);
      SetCaptureWindow(window_id);
    }
  }

  window_capturer_->Start(this);
  return 0;
}

bool BasicWindowCapturer::IsRunning() {
  return capture_thread_ && capture_thread_->IsRunning();
}

// Stop must be called on the same thread as Init as the underlying
// PlatformThread require that.
int32_t BasicWindowCapturer::StopCapture() {
  // Make sure invoke get passed through.
  worker_thread_->Invoke<void>(
      RTC_FROM_HERE, rtc::Bind(&BasicWindowCapturer::StopOnWorkerThread, this));
  return 0;
}

void BasicWindowCapturer::StopOnWorkerThread() {
  if (capture_thread_) {
    stopped_ = true;
    capture_started_ = false;
    capture_thread_->Stop();
    capture_thread_.reset();
  }
}

bool BasicWindowCapturer::CaptureStarted() {
  return capture_started_;
}

int32_t BasicWindowCapturer::CaptureSettings(
    webrtc::VideoCaptureCapability& settings) {
  settings.width = width_;
  settings.height = height_;
  settings.videoType = webrtc::VideoType::kI420;

  return 0;
}

int32_t BasicWindowCapturer::SetCaptureRotation(
    webrtc::VideoRotation rotation) {
  // Not implemented.
  return 0;
}

bool BasicWindowCapturer::GetCurrentWindowList(
    std::unordered_map<int, std::string>* window_list) {
  if (!window_capturer_) {
    RTC_LOG(LS_ERROR) << "No window capturer.";
    return false;
  }
  webrtc::DesktopCapturer::SourceList sources;
  bool have_source = window_capturer_->GetSourceList(&sources);
  if (!have_source) {
    RTC_LOG(LS_ERROR) << "No window available for capture";
    return false;
  }
  // Clear window_list if not empty.
  if (!window_list->empty()) {
    window_list->clear();
  }
  for (auto source : sources) {
    (*window_list)[source.id] = source.title;
  }
  return true;
}
bool BasicWindowCapturer::SetCaptureWindow(int window_id) {
  if (!window_capturer_) {
    RTC_LOG(LS_ERROR) << "No window capturer.";
    return false;
  }
  // Do we need to switch focus to this window? Currently we do.
  if (window_capturer_->SelectSource(window_id)) {
    source_specified_ = true;
    // Notify capture thread.
    window_capturer_->FocusOnSelectedSource();
    return true;
  }
  return false;
}
int BasicWindowCapturer::I420DataSize(int height,
                                      int stride_y,
                                      int stride_u,
                                      int stride_v) {
  return stride_y * height + (stride_u + stride_v) * ((height + 1) / 2);
}
void BasicWindowCapturer::AdjustFrameBuffer(int32_t width, int32_t height) {
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
// Executed in the context of BasicWindowCaptureThread.
void BasicWindowCapturer::CaptureFrame() {
  // invoke underlying desktop capture to capture one frame.
  if (!window_capturer_.get()) {
    RTC_LOG(LS_ERROR) << "Failed to capture one Window frame";
    return;
  }
  return window_capturer_->CaptureFrame();
}
void BasicWindowCapturer::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
    RTC_LOG(LS_ERROR) << "Failed to cpature one Window frame.";
    return;
  }

  if (!data_callback_)
    return;
  int32_t frame_width = frame->size().width();
  int32_t frame_height = frame->size().height();
  uint8_t* frame_data_rgba = frame->data();
  int frame_stride = frame->stride();
  // In case the window is not visible, capture returns frame of size 1x1 so
  // don't pass to stack.
  if (frame_width <= 1 || frame_height <= 1 || frame_data_rgba == nullptr) {
    RTC_LOG(LS_ERROR) << "Invalid Window data";
    return;
  }
  // On hosts with GEN & NV gfx co-existing, frame must be of even both
  // for width and height. So we will always scale for that.
  bool scale_required = false;
  int32_t old_frame_width = frame_width;
  int32_t old_frame_height = frame_height;
  int new_frame_stride = frame_stride;
  if (frame_width % 2 != 0) {
    frame_width -= 1;
    scale_required = true;
  }
  if (frame_height % 2 != 0) {
    frame_height -= 1;
    scale_required = true;
  }
  std::unique_ptr<uint8_t[]> new_frame_data_rgba;
  if (scale_required) {
    new_frame_data_rgba.reset(new uint8_t[frame_width * frame_height * 4]);
    new_frame_stride = frame_width * 4;
    libyuv::ARGBScale(frame_data_rgba, frame_stride, old_frame_width,
                      old_frame_height, new_frame_data_rgba.get(),
                      new_frame_stride, frame_width, frame_height,
                      libyuv::FilterMode::kFilterBilinear);
  }
  // The captured frame is of memory layout ABRG. convert it to I420 as
  // required.
  AdjustFrameBuffer(frame_width, frame_height);
  if (scale_required) {
    libyuv::ARGBToI420(new_frame_data_rgba.get(), new_frame_stride,
                       frame_buffer_->MutableDataY(), frame_buffer_->StrideY(),
                       frame_buffer_->MutableDataU(), frame_buffer_->StrideU(),
                       frame_buffer_->MutableDataV(), frame_buffer_->StrideV(),
                       frame_width, frame_height);
  } else {
    libyuv::ARGBToI420(frame_data_rgba, frame_stride,
                       frame_buffer_->MutableDataY(), frame_buffer_->StrideY(),
                       frame_buffer_->MutableDataU(), frame_buffer_->StrideU(),
                       frame_buffer_->MutableDataV(), frame_buffer_->StrideV(),
                       frame_width, frame_height);
  }
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
