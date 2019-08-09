// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OBJC_CAMERAVIDEOCAPTUREFACTORY_H_
#define OWT_BASE_OBJC_CAMERAVIDEOCAPTUREFACTORY_H_
#include <memory>
#include "talk/owt/sdk/base/objc/ObjcVideoCapturerInterface.h"
#include "talk/owt/sdk/include/cpp/owt/base/localcamerastreamparameters.h"
#include "third_party/webrtc/api/media_stream_interface.h"
#include "third_party/webrtc/api/scoped_refptr.h"
@class RTCVideoCapturer;
@class RTCVideoSource;
namespace owt {
namespace base {
/// Wraps an RTCVideoCapturer.
class CameraVideoCapturer : public ObjcVideoCapturerInterface {
 public:
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source() override;
  CameraVideoCapturer(RTCVideoCapturer* capturer, RTCVideoSource* source);
  ~CameraVideoCapturer() override;
 private:
  RTCVideoCapturer* capturer_;
  RTCVideoSource* source_;
};
}  // namespace base
}  // namespace woogeen
#endif  // OWT_BASE_OBJC_CAMERAVIDEOCAPTUREFACTORY_H_
