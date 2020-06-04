// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_CUSTOMIZEDVIDEOSOURCE_H
#define OWT_BASE_CUSTOMIZEDVIDEOSOURCE_H

#include "api/video/video_frame.h"
#include "api/video/video_rotation.h"
#include "api/video/video_sink_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "modules/video_capture/video_capture.h"
#include "owt/base/framegeneratorinterface.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/stream.h"
#include "owt/base/videoencoderinterface.h"
#include "pc/video_track_source.h"
#include "third_party/webrtc/api/media_stream_interface.h"
#include "third_party/webrtc/api/scoped_refptr.h"
#include "webrtc/api/scoped_refptr.h"

namespace owt {
namespace base {
using namespace cricket;

// Factory class for different customized capturers
class CustomizedVideoCapturerFactory {
 public:
  static rtc::scoped_refptr<webrtc::VideoCaptureModule> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer);
  static rtc::scoped_refptr<webrtc::VideoCaptureModule> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface* encoder);
#if defined(WEBRTC_WIN)
  static rtc::scoped_refptr<webrtc::VideoCaptureModule> Create(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer);
#endif
};

class CustomizedVideoSource
    : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
 public:
  CustomizedVideoSource();
  ~CustomizedVideoSource() override;

  void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                       const rtc::VideoSinkWants& wants) override;
  void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

 protected:
  void OnFrame(const webrtc::VideoFrame& frame);
  rtc::VideoSinkWants GetSinkWants();

 private:
  void UpdateVideoAdapter();

  rtc::VideoBroadcaster broadcaster_;
  cricket::VideoAdapter video_adapter_;
};

// The proxy capturer to actual VideoCaptureModule implementation.
class CustomizedCapturer : public CustomizedVideoSource,
                           public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  static CustomizedCapturer* Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer);
  static CustomizedCapturer* Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface* encoder);
#if defined(WEBRTC_WIN)
  static CustomizedCapturer* Create(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer);
#endif
  virtual ~CustomizedCapturer();

  // VideoSinkInterfaceImpl
  void OnFrame(const webrtc::VideoFrame& frame) override;

 private:
  CustomizedCapturer();
  bool Init(std::shared_ptr<LocalCustomizedStreamParameters> parameters,
            std::unique_ptr<VideoFrameGeneratorInterface> framer);
  bool Init(std::shared_ptr<LocalCustomizedStreamParameters> parameters,
            VideoEncoderInterface* encoder);
#if defined(WEBRTC_WIN)
  bool Init(std::shared_ptr<LocalDesktopStreamParameters> parameters,
            std::unique_ptr<LocalScreenStreamObserver> observer);
#endif
  void Destroy();

  rtc::scoped_refptr<webrtc::VideoCaptureModule> vcm_;
  webrtc::VideoCaptureCapability capability_;
};

// VideoTrackSources
class LocalRawCaptureTrackSource : public webrtc::VideoTrackSource {
 public:
  static rtc::scoped_refptr<LocalRawCaptureTrackSource> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer) {
    std::unique_ptr<CustomizedCapturer> capturer;
    capturer = absl::WrapUnique(
        CustomizedCapturer::Create(parameters, std::move(framer)));

    if (capturer)
      return new rtc::RefCountedObject<LocalRawCaptureTrackSource>(
          std::move(capturer));

    return nullptr;
  }

 protected:
  explicit LocalRawCaptureTrackSource(
      std::unique_ptr<CustomizedCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<CustomizedCapturer> capturer_;
};

#if defined(WEBRTC_WIN)
class LocalDesktopCaptureTrackSource : public webrtc::VideoTrackSource {
 public:
  static rtc::scoped_refptr<LocalDesktopCaptureTrackSource> Create(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer) {
    std::unique_ptr<CustomizedCapturer> capturer;
    capturer = absl::WrapUnique(
        CustomizedCapturer::Create(parameters, std::move(observer)));

    if (capturer)
      return new rtc::RefCountedObject<LocalDesktopCaptureTrackSource>(
          std::move(capturer));

    return nullptr;
  }

 protected:
  explicit LocalDesktopCaptureTrackSource(
      std::unique_ptr<CustomizedCapturer> capturer)
      : webrtc::VideoTrackSource(/*remote=*/false),
        capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<CustomizedCapturer> capturer_;
};
#endif

class LocalEncodedCaptureTrackSource : public webrtc::VideoTrackSource {
 public:
  static rtc::scoped_refptr<LocalEncodedCaptureTrackSource> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface* encoder) {
    std::unique_ptr<CustomizedCapturer> capturer;
    capturer =
        absl::WrapUnique(CustomizedCapturer::Create(parameters, encoder));

    if (capturer)
      return new rtc::RefCountedObject<LocalEncodedCaptureTrackSource>(
          std::move(capturer));

    return nullptr;
  }

 protected:
  explicit LocalEncodedCaptureTrackSource(
      std::unique_ptr<CustomizedCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<CustomizedCapturer> capturer_;
};

}  // namespace base
}  // namespace owt

#endif
