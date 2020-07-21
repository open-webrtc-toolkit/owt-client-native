// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/customizedframescapturer.h"
#include "talk/owt/sdk/base/desktopcapturer.h"
#include "talk/owt/sdk/base/customizedvideosource.h"

namespace owt {
namespace base {

rtc::scoped_refptr<webrtc::VideoCaptureModule>
CustomizedVideoCapturerFactory::Create(
    std::shared_ptr<LocalCustomizedStreamParameters> parameters,
    std::unique_ptr<VideoFrameGeneratorInterface> framer) {
  return new rtc::RefCountedObject<CustomizedFramesCapturer>(std::move(framer));
}

rtc::scoped_refptr<webrtc::VideoCaptureModule>
CustomizedVideoCapturerFactory::Create(
    std::shared_ptr<LocalCustomizedStreamParameters> parameters,
    VideoEncoderInterface* encoder) {
  return new rtc::RefCountedObject<CustomizedFramesCapturer>(
      parameters->ResolutionWidth(), parameters->ResolutionHeight(),
      parameters->Fps(), parameters->Bitrate(), encoder);
}

#if defined(WEBRTC_WIN)
rtc::scoped_refptr<webrtc::VideoCaptureModule>
CustomizedVideoCapturerFactory::Create(
    std::shared_ptr<LocalDesktopStreamParameters> parameters,
    std::unique_ptr<LocalScreenStreamObserver> observer) {
  webrtc::DesktopCaptureOptions options =
      webrtc::DesktopCaptureOptions::CreateDefault();
  options.set_allow_directx_capturer(true);
  if (parameters->SourceType() ==
      LocalDesktopStreamParameters::DesktopSourceType::kApplication) {
    return new rtc::RefCountedObject<BasicWindowCapturer>(options,
                                                          std::move(observer));
  } else {
    return new rtc::RefCountedObject<BasicScreenCapturer>(options);
  }
}
#endif

  CustomizedVideoSource::CustomizedVideoSource() = default;
  CustomizedVideoSource::~CustomizedVideoSource() = default;

  void CustomizedVideoSource::OnFrame(const webrtc::VideoFrame& frame) {
    // TODO(johny): We need to adapt frame for yuv input here, but not for
    // encoded input.
    broadcaster_.OnFrame(frame);
  }

  void CustomizedVideoSource::AddOrUpdateSink(
      rtc::VideoSinkInterface<webrtc::VideoFrame> * sink,
      const rtc::VideoSinkWants& wants) {
    broadcaster_.AddOrUpdateSink(sink, wants);
    UpdateVideoAdapter();
  }
  void CustomizedVideoSource::RemoveSink(
      rtc::VideoSinkInterface<webrtc::VideoFrame> * sink) {
    broadcaster_.RemoveSink(sink);
    UpdateVideoAdapter();
  }

  rtc::VideoSinkWants CustomizedVideoSource::GetSinkWants() {
    return broadcaster_.wants();
  }

  void CustomizedVideoSource::UpdateVideoAdapter() {
    // NOT Implemented.
  }

  CustomizedCapturer* CustomizedCapturer::Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer) {
    std::unique_ptr<CustomizedCapturer> vcm_capturer(new CustomizedCapturer());
    if (!vcm_capturer->Init(parameters, std::move(framer)))
      return nullptr;
    return vcm_capturer.release();
  }

  CustomizedCapturer* CustomizedCapturer::Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface * encoder) {
    std::unique_ptr<CustomizedCapturer> vcm_capturer(new CustomizedCapturer());
    if (!vcm_capturer->Init(parameters, encoder))
      return nullptr;
    return vcm_capturer.release();
  }

#if defined(WEBRTC_WIN)
  CustomizedCapturer* CustomizedCapturer::Create(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer) {
    std::unique_ptr<CustomizedCapturer> vcm_capturer(new CustomizedCapturer());
    if (!vcm_capturer->Init(parameters, std::move(observer)))
      return nullptr;
    return vcm_capturer.release();
  }
#endif

  CustomizedCapturer::CustomizedCapturer() : vcm_(nullptr) {}

  bool CustomizedCapturer::Init(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer) {
    vcm_ =
        CustomizedVideoCapturerFactory::Create(parameters, std::move(framer));

    if (!vcm_)
      return false;

    vcm_->RegisterCaptureDataCallback(this);
    capability_.width = parameters->ResolutionWidth();
    capability_.height = parameters->ResolutionHeight();
    capability_.maxFPS = parameters->Fps();
    capability_.videoType = webrtc::VideoType::kI420;

    if (vcm_->StartCapture(capability_) != 0) {
      Destroy();
      return false;
    }

    RTC_CHECK(vcm_->CaptureStarted());
    return true;
  }


  bool CustomizedCapturer::Init(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface * encoder) {
    vcm_ = CustomizedVideoCapturerFactory::Create(parameters, encoder);

    if (!vcm_)
      return false;

    vcm_->RegisterCaptureDataCallback(this);
    capability_.width = parameters->ResolutionWidth();
    capability_.height = parameters->ResolutionHeight();
    capability_.maxFPS = parameters->Fps();

    if (vcm_->StartCapture(capability_) != 0) {
      Destroy();
      return false;
    }

    RTC_CHECK(vcm_->CaptureStarted());
    return true;
  }

#if defined(WEBRTC_WIN)
  bool CustomizedCapturer::Init(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer) {
    vcm_ =
        CustomizedVideoCapturerFactory::Create(parameters, std::move(observer));

    if (!vcm_)
      return false;

    vcm_->RegisterCaptureDataCallback(this);
    capability_.maxFPS = parameters->Fps();
    capability_.videoType = webrtc::VideoType::kI420;

    if (vcm_->StartCapture(capability_) != 0) {
      Destroy();
      return false;
    }

    RTC_CHECK(vcm_->CaptureStarted());
    return true;
  }
#endif

  void CustomizedCapturer::Destroy() {
    if (!vcm_)
      return;

    vcm_->StopCapture();
    vcm_->DeRegisterCaptureDataCallback();
    vcm_ = nullptr;
  }

  CustomizedCapturer::~CustomizedCapturer() { Destroy(); }

  void CustomizedCapturer::OnFrame(const webrtc::VideoFrame& frame) {
    CustomizedVideoSource::OnFrame(frame);
  }

}  // namespace base
}  // namespace base
