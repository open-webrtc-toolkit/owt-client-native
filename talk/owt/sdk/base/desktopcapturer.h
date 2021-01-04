// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_BASICSCREENCAPTURER_H_
#define OWT_BASE_BASICSCREENCAPTURER_H_
#include <string>
#include <unordered_map>
#include <vector>
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
#include <memory>
#endif
#include "modules/video_capture/video_capture.h"
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "webrtc/modules/desktop_capture/desktop_capturer.h"
#include "webrtc/modules/desktop_capture/desktop_frame.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/platform_thread.h"
#include "webrtc/rtc_base/stream.h"
#include "webrtc/rtc_base/string_utils.h"
#include "webrtc/rtc_base/synchronization/mutex.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"

namespace owt {
namespace base {
class ScreenCaptureThread : public rtc::Thread {
 public:
  ScreenCaptureThread() : rtc::Thread(rtc::SocketServer::CreateDefault()) {}
  virtual void Run() override;
  ~ScreenCaptureThread() override;
};

// Base class for screen/window capturer.
class BasicDesktopCapturer : public webrtc::VideoCaptureModule,
                             public webrtc::DesktopCapturer::Callback {
 public:
  BasicDesktopCapturer() {}
  virtual ~BasicDesktopCapturer() {}

  virtual void RegisterCaptureDataCallback(
      rtc::VideoSinkInterface<webrtc::VideoFrame>* dataCallback) override {
    webrtc::MutexLock lock(&datacb_lock_);
    data_callback_ = dataCallback;
  }
  virtual void DeRegisterCaptureDataCallback() override {
    webrtc::MutexLock lock(&datacb_lock_);
    data_callback_ = nullptr;
  }

  virtual int32_t StartCapture(
      const webrtc::VideoCaptureCapability& capability) override {
    return 0;
  }
  virtual int32_t StopCapture() override { return 0; }
  virtual const char* CurrentDeviceName() const override {
    return "ScreenCapturer";
  }
  virtual bool CaptureStarted() override { return false; }
  virtual int32_t CaptureSettings(
      webrtc::VideoCaptureCapability& settings) override {
    return 0;
  }
  virtual int32_t SetCaptureRotation(webrtc::VideoRotation rotation) override {
    return 0;
  }
  virtual bool SetApplyRotation(bool enable) override { return false; }
  virtual bool GetApplyRotation() override { return false; }
  virtual bool IsRunning() { return false; }
  virtual void OnCaptureResult(
      webrtc::DesktopCapturer::Result result,
      std::unique_ptr<webrtc::DesktopFrame> frame) override{}
  virtual bool GetCurrentWindowList(
      std::unordered_map<int, std::string>* window_list) {
    return false;
  }
  virtual bool SetCaptureWindow(int window_id) { return false; }

 public:
  rtc::VideoSinkInterface<webrtc::VideoFrame>* data_callback_;
  webrtc::Mutex datacb_lock_;
};
// Capturer for capturing from local screen. Currently it uses
// basic desktop frame instead of shared memory for storing captured frame.
// The frame captured by WebRTC stack is of format RGBA and BasicScreenCapture
// will convert it to I420 and signal stack of the frame.
class BasicScreenCapturer : public BasicDesktopCapturer {
 public:
  BasicScreenCapturer(webrtc::DesktopCaptureOptions options);
  virtual ~BasicScreenCapturer();
  virtual int32_t StartCapture(
      const webrtc::VideoCaptureCapability& capability) override;
  virtual int32_t StopCapture() override;
  virtual bool CaptureStarted() override;
  virtual int32_t CaptureSettings(webrtc::VideoCaptureCapability& settings) override;
  virtual int32_t SetCaptureRotation(webrtc::VideoRotation rotation) override;
  // Override virtual methods of parent class VideoCapturer.
  virtual bool IsRunning() override;
  // DesktopCapturer::Callback implementation
  virtual void OnCaptureResult(
      webrtc::DesktopCapturer::Result result,
      std::unique_ptr<webrtc::DesktopFrame> frame) override;

 protected:
 private:
  class BasicScreenCaptureThread;  // Forward declaration, defined in .cc.
  int I420DataSize(int height, int stride_y, int stride_u, int stride_v);
  void CaptureFrame();
  void AdjustFrameBuffer(int32_t width, int32_t height);
  std::unique_ptr<BasicScreenCaptureThread> screen_capture_thread_;
  int width_;
  int height_;
  uint32_t frame_buffer_capacity_;
  rtc::scoped_refptr<webrtc::I420Buffer>
      frame_buffer_;  // Reuseable buffer for video frames.
  std::unique_ptr<webrtc::DesktopCapturer> screen_capturer_;
  webrtc::DesktopCaptureOptions screen_capture_options_;
  bool capture_started_ = false;
  webrtc::Mutex lock_;
  RTC_DISALLOW_COPY_AND_ASSIGN(BasicScreenCapturer);
};
// Capturer for capturing from specified window. Once the capturer is created,
// User has to call GetWindowList() to enumerate current available windows, and
// Then select window to be used by calling SetCaptureWindow() to actually start
// the capture.
class BasicWindowCapturer : public BasicDesktopCapturer {
 public:
  BasicWindowCapturer(webrtc::DesktopCaptureOptions options,
                      std::unique_ptr<LocalScreenStreamObserver> observer);
  virtual ~BasicWindowCapturer();

  // Override virtual methods of parent class VideoCapturer.
  virtual int32_t StartCapture(
      const webrtc::VideoCaptureCapability& capability) override;
  virtual int32_t StopCapture() override;

  virtual bool CaptureStarted() override;
  virtual int32_t CaptureSettings(webrtc::VideoCaptureCapability& settings) override;
  virtual int32_t SetCaptureRotation(webrtc::VideoRotation rotation) override;
  virtual bool IsRunning() override;

  // DesktopCapturer::Callback implementation
  void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                       std::unique_ptr<webrtc::DesktopFrame> frame) final;
  bool GetCurrentWindowList(
      std::unordered_map<int, std::string>* window_list) override;
  bool SetCaptureWindow(int window_id) override;
  enum MessageType : int {
    kMessageTypeWindowSpecified = 1,
    kMessageTypeContinue
  };

 protected:
  // Override virtual methods of parent class VideoCapturer.
 private:
  class BasicWindowCaptureThread;  // Forward declaration, defined in .cc.
  static void WindowCaptureThreadFunc(void* param);
  bool CaptureThreadProcess();
  void InitOnWorkerThread();
  void StopOnWorkerThread();
  int I420DataSize(int height, int stride_y, int stride_u, int stride_v);
  void CaptureFrame();
  void AdjustFrameBuffer(int32_t width, int32_t height);
  // BasicWindowCaptureThread* window_capture_thread_;
  int width_;
  int height_;
  uint32_t frame_buffer_capacity_;
  rtc::scoped_refptr<webrtc::I420Buffer>
      frame_buffer_;  // Reuseable buffer for video frames.
  std::unique_ptr<rtc::PlatformThread> capture_thread_;
  std::unique_ptr<owt::base::ScreenCaptureThread>
      worker_thread_;  // Set in Start(), unset in Stop();
  std::unique_ptr<webrtc::DesktopCapturer> window_capturer_;
  webrtc::DesktopCaptureOptions window_capture_options_;
  bool source_specified_;
  uint64_t last_call_record_millis_;
  webrtc::Clock* clock_;
  int64_t need_sleep_ms_;
  int64_t real_sleep_ms_;
  bool capturing_;
  bool stopped_;
  bool capture_started_ = false;
  bool quit_;
  webrtc::Mutex lock_;
  std::unique_ptr<LocalScreenStreamObserver> observer_;
  RTC_DISALLOW_COPY_AND_ASSIGN(BasicWindowCapturer);
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_BASICSCREENCAPTURER_H_
