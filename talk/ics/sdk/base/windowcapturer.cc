/*
 * Intel License
 */

#include <iostream>
#include <mutex>
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
// Definition of private class BasicWindowCaptureThread that periodically
// generates frames.
///////////////////////////////////////////////////////////////////////
class BasicWindowCapturer::BasicWindowCaptureThread
    : public rtc::Thread,
      public rtc::MessageHandler {
 public:
  explicit BasicWindowCaptureThread(BasicWindowCapturer* capturer)
      : capturer_(capturer), finished_(false), source_specified_(false) {
    waiting_time_ms_ = 1000 / 30;
  }

  virtual ~BasicWindowCaptureThread() { Stop(); }

  // Override virtual method of parent Thread. Context: Worker Thread.
  virtual void Run() {
    // Read the first frame and start the message pump. The pump runs until
    // Stop() is called externally or Quit() is called by OnMessage().
    bool has_source = false;
    if (capturer_) {
      {
        std::lock_guard<std::mutex> lock(window_source_mutex_);
        has_source = source_specified_;
      }
      if (has_source)
        capturer_->CaptureFrame();
      rtc::Thread::Current()->Post(RTC_FROM_HERE, this, kMessageTypeContinue);
      rtc::Thread::Current()->ProcessMessages(kForever);
    }

    rtc::CritScope cs(&crit_);
    finished_ = true;
  }

  virtual void OnMessage(rtc::Message* msg) {
    switch (msg->message_id) {
      case kMessageTypeContinue:
        if (capturer_) {
          {
            std::lock_guard<std::mutex> lock(window_source_mutex_);
            if (source_specified_)
              capturer_->CaptureFrame();
          }
          rtc::Thread::Current()->PostDelayed(RTC_FROM_HERE, waiting_time_ms_,
                                              this, kMessageTypeContinue);
        } else {
          rtc::Thread::Current()->Quit();
        }
        break;
      case kMessageTypeWindowSpecified: {
        std::lock_guard<std::mutex> lock(window_source_mutex_);
        source_specified_ = true;
      } break;
      default:
        break;
    }
  }

  // Check if Run() is finished.
  bool Finished() const {
    rtc::CritScope cs(&crit_);
    return finished_;
  }

 private:
  BasicWindowCapturer* capturer_;
  mutable rtc::CriticalSection crit_;
  bool finished_;
  bool source_specified_;
  mutable std::mutex window_source_mutex_;
  int waiting_time_ms_;

  RTC_DISALLOW_COPY_AND_ASSIGN(BasicWindowCaptureThread);
};

/////////////////////////////////////////////////////////////////////
// Implementation of class BasicWindowCapturer. Window capturer is
// different from window capturer as the window may be disposed
// at any time.
/////////////////////////////////////////////////////////////////////

BasicWindowCapturer::BasicWindowCapturer(webrtc::DesktopCaptureOptions options)
    : window_capture_thread_(nullptr),
      width_(0),
      height_(0),
      frame_buffer_capacity_(0),
      frame_buffer_(nullptr),
      async_invoker_(nullptr),
      window_capture_options_(options),
      source_specified_(false) {
  window_capturer_ =
      webrtc::DesktopCapturer::CreateWindowCapturer(window_capture_options_);
}

BasicWindowCapturer::~BasicWindowCapturer() {
  Stop();
}

void BasicWindowCapturer::Init() {
  // Only I420 frame is supported. RGBA frame is not supported here.
  VideoFormat format(width_, height_, VideoFormat::kMinimumInterval,
                     cricket::FOURCC_I420);
  std::vector<VideoFormat> supported;
  supported.push_back(format);
  SetSupportedFormats(supported);
}

CaptureState BasicWindowCapturer::Start(const VideoFormat& capture_format) {
  if (IsRunning()) {
    LOG(LS_ERROR) << "Basic Window Capturerer is already running";
    return CS_FAILED;
  }
  if (!window_capturer_.get()) {
    LOG(LS_ERROR) << "Desktop capturer creation failed, not able to start it";
    return CS_FAILED;
  }
  SetCaptureFormat(&capture_format);
  window_capturer_->Start(this);

  worker_thread_ = rtc::Thread::Current();
  RTC_DCHECK(!async_invoker_);
  async_invoker_.reset(new rtc::AsyncInvoker());
  window_capture_thread_ = new BasicWindowCaptureThread(this);
  bool ret = window_capture_thread_->Start();
  if (ret) {
    LOG(LS_INFO) << "Window capture thread started";
    return CS_RUNNING;
  } else {
    async_invoker_.reset();
    worker_thread_ = nullptr;
    LOG(LS_ERROR) << "Window capture thread failed to start";
    return CS_FAILED;
  }
}

bool BasicWindowCapturer::IsRunning() {
  return window_capture_thread_ && !window_capture_thread_->Finished();
}

void BasicWindowCapturer::Stop() {
  if (window_capture_thread_) {
    window_capture_thread_->Quit();
    window_capture_thread_ = nullptr;
    LOG(LS_INFO) << "Window capture thread stopped";
  }
  SetCaptureFormat(nullptr);
  worker_thread_ = nullptr;
  async_invoker_.reset();
}

bool BasicWindowCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs) {
  if (!fourccs) {
    return false;
  }
  fourccs->push_back(GetSupportedFormats()->at(0).fourcc);
  return true;
}

bool BasicWindowCapturer::GetCurrentWindowList(
    std::unordered_map<int, std::string>* window_list) {
  if (!window_capturer_) {
    LOG(LS_ERROR) << "No window capturer.";
    return false;
  }
  webrtc::DesktopCapturer::SourceList sources;
  bool have_source = window_capturer_->GetSourceList(&sources);

  if (!have_source) {
    LOG(LS_ERROR) << "No window available for capture";
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
    LOG(LS_ERROR) << "No window capturer.";
    return false;
  }

  // Do we need to switch focus to this window? Currently we do.
  if (window_capturer_->SelectSource(window_id)) {
    source_specified_ = true;
    // Notify capture thread.
    window_capturer_->FocusOnSelectedSource();
    window_capture_thread_->Post(RTC_FROM_HERE, window_capture_thread_,
                                 kMessageTypeWindowSpecified, nullptr);
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

// Executed in the context of BasicWindowCaptureThread.
void BasicWindowCapturer::CaptureFrame() {
  // invoke underlying desktop capture to capture one frame.
  if (!window_capturer_.get()) {
    LOG(LS_ERROR) << "Failed to capture one Window frame";
    return;
  }
  return window_capturer_->CaptureFrame();
}

void BasicWindowCapturer::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
    LOG(LS_ERROR) << "Failed to cpature one Window frame.";
    return;
  }

  int32_t frame_width = frame->size().width();
  int32_t frame_height = frame->size().height();
  uint8_t* frame_data_rgba = frame->data();
  int frame_stride = frame->stride();
  // In case the window is not visible, capture returns frame of size 1x1 so
  // don't pass to stack.
  if (frame_width <= 1 || frame_height <= 1 || frame_data_rgba == nullptr) {
    LOG(LS_ERROR) << "Invalid Window data";
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
