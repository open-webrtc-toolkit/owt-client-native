// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/base/objc/AVFoundationVideoCapturerFactory.h"
#import "webrtc/sdk/objc/Framework/Classes/avfoundationvideocapturer.h"
namespace owt {
namespace base {
std::unique_ptr<cricket::VideoCapturer> AVFoundationVideoCapturerFactory::Create(
    const LocalCameraStreamParameters& parameters) {
  std::unique_ptr<webrtc::AVFoundationVideoCapturer> av_foundation_capturer(
      new webrtc::AVFoundationVideoCapturer());
  if ((parameters.CameraId() == "com.apple.avfoundation.avcapturedevice.built-in_video:0" ||
       parameters.CameraId() == "Back Camera") &&
      av_foundation_capturer->CanUseBackCamera()) {
    av_foundation_capturer->SetUseBackCamera(true);
  } else if (parameters.CameraId() ==
                 "com.apple.avfoundation.avcapturedevice.built-in_video:"
                 "1" ||
             parameters.CameraId() == "Front Camera") {
    av_foundation_capturer->SetUseBackCamera(false);
  } else {
    return nullptr;
  }
  return std::move(av_foundation_capturer);
}
}
}
