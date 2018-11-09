/*
 * Intel License
 */
#ifndef OMS_BASE_BASOMSCREENCAPTURER_H_
#define OMS_BASE_BASOMSCREENCAPTURER_H_
#include <string>
#include <vector>
#include <unordered_map>
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
#include <memory>
#endif
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/media/base/videocapturer.h"
#include "webrtc/rtc_base/stream.h"
#include "webrtc/rtc_base/stringutils.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/asyncinvoker.h"
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/platform_thread.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "webrtc/modules/desktop_capture/desktop_capturer.h"
#include "webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "webrtc/modules/desktop_capture/desktop_frame.h"
#include "talk/oms/sdk/include/cpp/oms/base/stream.h"
namespace oms {
namespace base {
class ScreenCaptureThread : public rtc::Thread {
 public:
  virtual void Run() override;
  ~ScreenCaptureThread() override;
};
using namespace cricket;
// Base class for screen/window capturer.
class BasicDesktopCapturer : public VideoCapturer,
                             public webrtc::DesktopCapturer::Callback {
 public:
  BasicDesktopCapturer() {}
  virtual ~BasicDesktopCapturer() {}
  virtual void Init() {}
  virtual CaptureState Start(
      const cricket::VideoFormat& capture_format) override {
    return CS_RUNNING;
  };
  virtual void Stop() override{};
  virtual bool IsRunning() override { return false; };
  virtual bool IsScreencast() const override { return true; }
  virtual void OnCaptureResult(
      webrtc::DesktopCapturer::Result result,
      std::unique_ptr<webrtc::DesktopFrame> frame) override{};
  virtual bool GetCurrentWindowList(
      std::unordered_map<int, std::string>* window_list) {
    return false;
  };
  virtual bool SetCaptureWindow(int window_id) { return false; };
};
// Capturer for capturing from local screen. Currently it uses
// basic desktop frame instead of shared memory for storing captured frame.
// The frame captured by WebRTC stack is of format RGBA and BasicScreenCapture
// will convert it to I420 and signal stack of the frame.
class BasicScreenCapturer : public BasicDesktopCapturer {
 public:
  BasicScreenCapturer(webrtc::DesktopCaptureOptions options);
  virtual ~BasicScreenCapturer();
  virtual void Init() override;
  // Override virtual methods of parent class VideoCapturer.
  virtual CaptureState Start(
      const cricket::VideoFormat& capture_format) override;
  virtual void Stop() override;
  virtual bool IsRunning() override;
  virtual bool IsScreencast() const override { return false; }
  // DesktopCapturer::Callback implementation
  virtual void OnCaptureResult(
      webrtc::DesktopCapturer::Result result,
      std::unique_ptr<webrtc::DesktopFrame> frame) override;
 protected:
  // Override virtual methods of parent class VideoCapturer.
  virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
 private:
  class BasicScreenCaptureThread;  // Forward declaration, defined in .cc.
  int I420DataSize(int height, int stride_y, int stride_u, int stride_v);
  void CaptureFrame();
  void AdjustFrameBuffer(int32_t width, int32_t height);
  BasicScreenCaptureThread* screen_capture_thread_;
  int width_;
  int height_;
  uint32_t frame_buffer_capacity_;
  rtc::scoped_refptr<webrtc::I420Buffer>
      frame_buffer_;  // Reuseable buffer for video frames.
  // Consider to use NativeHandleBuffer if you want to support encoded frame.
  rtc::Thread* worker_thread_;  // Set in Start(), unset in Stop();
  std::unique_ptr<rtc::AsyncInvoker> async_invoker_;
  std::unique_ptr<webrtc::DesktopCapturer> screen_capturer_;
  webrtc::DesktopCaptureOptions screen_capture_options_;
  rtc::CriticalSection lock_;
  RTC_DISALLOW_COPY_AND_ASSIGN(BasicScreenCapturer);
};
// Capturer for capturing from specified window. Once the capturer is created,
// User has to call GetWindowList() to enumerate current available windows, and
// Then select window to be used by calling SetCaptureWindow() to actually start
// the capture.
class BasicWindowCapturer : public BasicDesktopCapturer {
 public:
  BasicWindowCapturer(webrtc::DesktopCaptureOptions options, std::unique_ptr<LocalScreenStreamObserver> observer);
  virtual ~BasicWindowCapturer();
  void Init() override;
  // Override virtual methods of parent class VideoCapturer.
  virtual CaptureState Start(
      const cricket::VideoFormat& capture_format) override;
  virtual void Stop() override;
  virtual bool IsRunning() override;
  virtual bool IsScreencast() const override { return true; }
  // DesktopCapturer::Callback implementation
  void OnCaptureResult(webrtc::DesktopCapturer::Result result,
                       std::unique_ptr<webrtc::DesktopFrame> frame) final;
  bool GetCurrentWindowList(std::unordered_map<int, std::string>* window_list) override;
  bool SetCaptureWindow(int window_id) override;
  enum MessageType : int {
    kMessageTypeWindowSpecified = 1,
    kMessageTypeContinue
  };
 protected:
  // Override virtual methods of parent class VideoCapturer.
  virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
 private:
  class BasicWindowCaptureThread;  // Forward declaration, defined in .cc.
  static bool WindowCaptureThreadFunc(void* param);
  bool CaptureThreadProcess();
  void InitOnWorkerThread();
  void StopOnWorkerThread();
  int I420DataSize(int height, int stride_y, int stride_u, int stride_v);
  void CaptureFrame();
  void AdjustFrameBuffer(int32_t width, int32_t height);
  //BasicWindowCaptureThread* window_capture_thread_;
  int width_;
  int height_;
  uint32_t frame_buffer_capacity_;
  rtc::scoped_refptr<webrtc::I420Buffer>
      frame_buffer_;            // Reuseable buffer for video frames.
  std::unique_ptr<rtc::PlatformThread> capture_thread_;
  std::unique_ptr<oms::base::ScreenCaptureThread> worker_thread_;  // Set in Start(), unset in Stop();
  std::unique_ptr<rtc::AsyncInvoker> async_invoker_;
  std::unique_ptr<webrtc::DesktopCapturer> window_capturer_;
  webrtc::DesktopCaptureOptions window_capture_options_;
  bool source_specified_;
  uint64_t last_call_record_millis_;
  webrtc::Clock* clock_;
  int64_t need_sleep_ms_;
  int64_t real_sleep_ms_;
  bool capturing_;
  bool stopped_;
  rtc::CriticalSection lock_;
  std::unique_ptr<LocalScreenStreamObserver> observer_;
  RTC_DISALLOW_COPY_AND_ASSIGN(BasicWindowCapturer);
};
}  // namespace base
}  // namespace oms
#endif  // OMS_BASE_BASOMSCREENCAPTURER_H_
