// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_STREAM_FACTORY_H_
#define OWT_BASE_STREAM_FACTORY_H_
#include <unordered_map>
#inlcude "owt/base/stream.h"
#include "owt/base/exception.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/macros.h"
#include "owt/base/videoencoderinterface.h"
#include "owt/base/videorendererinterface.h"
namespace webrtc {
  class MediaStreamInterface;
  class VideoTrackSourceInterface;
}
namespace owt {
namespace base {
using webrtc::MediaStreamInterface;
/// Factory class for creating all types of media streams. Not implemented for Windows.
class StreamFactory {
 public:
  static std::shared_ptr<LocalStream> CreateLocalStream(MediaStreamDeviceConstraints constraints);
  static std::shared_ptr<LocalStream> CreateLocalStream(MediaStreamScreencastConstraints constraints);
  static std::shared_ptr<LocalStream> CreateLocalStream(MediaStreamCustomizedConstraints constraints);
};
} // namespace base
} // namespace owt
#endif  // OWT_BASE_STREAM_FACTORY_H_
