// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OBJC_OBJCVIDEOCAPTUREINTERFACE_H_
#define OWT_BASE_OBJC_OBJCVIDEOCAPTUREINTERFACE_H_
#include "talk/owt/sdk/include/cpp/owt/base/localcamerastreamparameters.h"
#include "third_party/webrtc/api/scoped_refptr.h"
#include "third_party/webrtc/api/media_stream_interface.h"
namespace owt {
namespace base {
/**
  @brief Video capturer interface for ObjC implementation.
  @details It's too complex to implement a cricket::VideoCapturer for ObjC video
  capturers. However, ObjcVideoTrackSource does not hold an reference to
  capturer. So we need to hold a reference in C++ code to avoid capturer being
  destroyed.
  */
class ObjcVideoCapturerInterface {
 public:
  virtual rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source() = 0;
  virtual ~ObjcVideoCapturerInterface(){}
};
class ObjcVideoCapturerFactory {
 public:
  static std::unique_ptr<ObjcVideoCapturerInterface> Create(
      const LocalCameraStreamParameters& parameters);
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_OBJC_OBJCVIDEOCAPTUREINTERFACE_H_
