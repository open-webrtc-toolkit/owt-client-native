/*
 * Intel License
 */

#ifndef ICS_BASE_CUSTOMIZEDFRAMESCAPTURER_H_
#define ICS_BASE_CUSTOMIZEDFRAMESCAPTURER_H_

#include <string>
#include <vector>
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
#include "ics/base/framegeneratorinterface.h"
#include "ics/base/videoencoderinterface.h"

namespace ics {
namespace base {
using namespace cricket;
// Simulated video capturer that periodically reads frames from a file.
class CustomizedFramesCapturer : public VideoCapturer {
 public:
  CustomizedFramesCapturer(std::unique_ptr<VideoFrameGeneratorInterface> rawFrameGenerator);
  CustomizedFramesCapturer(int width, int height, int fps, int bitrate_kbps, VideoEncoderInterface* encoder);
  virtual ~CustomizedFramesCapturer();
  static const char* kRawFrameDeviceName;

  void Init();
  // Override virtual methods of parent class VideoCapturer.
  virtual CaptureState Start(
      const cricket::VideoFormat& capture_format) override;
  virtual void Stop() override;
  virtual bool IsRunning() override;
  virtual bool IsScreencast() const override { return false; }

 protected:
  // Override virtual methods of parent class VideoCapturer.
  virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;

  // Read a frame and determine how long to wait for the next frame.
  virtual void ReadFrame();
  // Adjust |frame_buffer_|'s capacity to store frame data. |frame_buffer_|'s
  // capacity should be greater or equal to |size|.
  virtual void AdjustFrameBuffer(uint32_t size);

 private:
  class CustomizedFramesThread;  // Forward declaration, defined in .cc.

  int I420DataSize(int height, int stride_y, int stride_u, int stride_v);

  std::unique_ptr<VideoFrameGeneratorInterface> frame_generator_;
  VideoEncoderInterface* encoder_;
  CustomizedFramesThread* frames_generator_thread;
  int width_;
  int height_;
  int fps_;
  int bitrate_kbps_;
  VideoFrameGeneratorInterface::VideoFrameCodec frame_type_;
  uint32_t frame_buffer_capacity_;
  rtc::scoped_refptr<webrtc::I420Buffer> frame_buffer_; // Reuseable buffer for video frames.
  // Consider to use NativeHandleBuffer if you want to support encoded frame.
  rtc::Thread* worker_thread_;  // Set in Start(), unset in Stop();
  std::unique_ptr<rtc::AsyncInvoker> async_invoker_;
  rtc::CriticalSection lock_;

  RTC_DISALLOW_COPY_AND_ASSIGN(CustomizedFramesCapturer);
};

}  // namespace base
}  // namespace ics

#endif  // ICS_BASE_CUSTOMIZEDFRAMESCAPTURER_H_
