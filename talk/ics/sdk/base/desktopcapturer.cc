/*
 * Intel License
 */

#include <iostream>
#include "libyuv/convert.h"
#include "webrtc/rtc_base/bytebuffer.h"
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/aligned_malloc.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "talk/ics/sdk/base/desktopcapturer.h"

namespace ics {
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
      : capturer_(capturer), finished_(false) {
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

    rtc::CritScope cs(&crit_);
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
    rtc::CritScope cs(&crit_);
    return finished_;
  }

 private:
  BasicScreenCapturer* capturer_;
  mutable rtc::CriticalSection crit_;
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
      async_invoker_(nullptr),
      screen_capture_options_(options) {
  screen_capturer_ =
      webrtc::DesktopCapturer::CreateScreenCapturer(screen_capture_options_);
}

BasicScreenCapturer::~BasicScreenCapturer() {
  Stop();
}

void BasicScreenCapturer::Init() {
  // Only I420 frame is supported. RGBA frame is not supported here.
  cricket::VideoFormat format(width_, height_, cricket::VideoFormat::kMinimumInterval,
                     cricket::FOURCC_I420);
  std::vector<cricket::VideoFormat> supported;
  supported.push_back(format);
  SetSupportedFormats(supported);
}

CaptureState BasicScreenCapturer::Start(const cricket::VideoFormat& capture_format) {
  if (IsRunning()) {
    LOG(LS_ERROR) << "Basic Screen Capturerer is already running";
    return CS_FAILED;
  }
  if (!screen_capturer_.get()) {
    LOG(LS_ERROR) << "Desktop capturer creation failed, not able to start it";
    return CS_FAILED;
  }
  SetCaptureFormat(&capture_format);
  screen_capturer_->Start(this);

  worker_thread_ = rtc::Thread::Current();
  RTC_DCHECK(!async_invoker_);
  async_invoker_.reset(new rtc::AsyncInvoker());
  screen_capture_thread_ = new BasicScreenCaptureThread(this);
  bool ret = screen_capture_thread_->Start();
  if (ret) {
    LOG(LS_INFO) << "Screen capture thread started";
    return CS_RUNNING;
  } else {
    async_invoker_.reset();
    worker_thread_ = nullptr;
    LOG(LS_ERROR) << "Screen capture thread failed to start";
    return CS_FAILED;
  }
}

bool BasicScreenCapturer::IsRunning() {
  return screen_capture_thread_ && !screen_capture_thread_->Finished();
}

void BasicScreenCapturer::Stop() {
  if (screen_capture_thread_) {
    screen_capture_thread_->Quit();
    screen_capture_thread_ = NULL;
    LOG(LS_INFO) << "Screen capture thread stopped";
  }
  SetCaptureFormat(NULL);
  worker_thread_ = nullptr;
  async_invoker_.reset();
}

bool BasicScreenCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs) {
  if (!fourccs) {
    return false;
  }
  fourccs->push_back(GetSupportedFormats()->at(0).fourcc);
  return true;
}

int BasicScreenCapturer::I420DataSize(int height,
                                      int stride_y,
                                      int stride_u,
                                      int stride_v) {
  return stride_y * height + (stride_u + stride_v) * ((height + 1) / 2);
}

void BasicScreenCapturer::AdjustFrameBuffer(int32_t width, int32_t height) {
  if (width_ != width || height != height_ || !frame_buffer_) {
    LOG(LS_VERBOSE) << "Allocate new memory for frame buffer.";
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
    LOG(LS_ERROR) << "Failed to capture one screen frame";
    return;
  }
  return screen_capturer_->CaptureFrame();
}

void BasicScreenCapturer::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
    LOG(LS_ERROR) << "Failed to cpature one screen frame.";
    return;
  }

  int32_t frame_width = frame->size().width();
  int32_t frame_height = frame->size().height();
  uint8_t* frame_data_rgba = frame->data();
  int frame_stride = frame->stride();
  if (frame_width == 0 || frame_height == 0 || frame_data_rgba == nullptr) {
    LOG(LS_ERROR) << "Invalid screen data";
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

  webrtc::VideoFrame capturedFrame(frame_buffer_, 0, rtc::TimeMillis(),
                                   webrtc::kVideoRotation_0);
  OnFrame(capturedFrame, frame_width, frame_height);
}

}  // namespace base
}  // namespace ics
