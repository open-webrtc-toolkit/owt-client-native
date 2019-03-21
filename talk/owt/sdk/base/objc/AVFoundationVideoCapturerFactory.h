// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
#define WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
#include "talk/owt/sdk/include/cpp/owt/base/localcamerastreamparameters.h"
#include "webrtc/media/base/videocapturer.h"
namespace owt {
namespace base {
class AVFoundationVideoCapturerFactory {
 public:
  static std::unique_ptr<cricket::VideoCapturer> Create(
      const LocalCameraStreamParameters& parameters);
};
}
}
#endif  // WOOGEEN_BASE_AVFOUNDATIONVIDEOCAPTURERFACTORY_H_
