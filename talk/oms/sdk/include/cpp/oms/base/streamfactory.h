// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_STREAM_FACTORY_H_
#define OMS_BASE_STREAM_FACTORY_H_
#include <unordered_map>
#inlcude "oms/base/stream.h"
#include "oms/base/exception.h"
#include "oms/base/localcamerastreamparameters.h"
#include "oms/base/macros.h"
#include "oms/base/videoencoderinterface.h"
#include "oms/base/videorendererinterface.h"
namespace webrtc {
  class MediaStreamInterface;
  class VideoTrackSourceInterface;
}
namespace oms {
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
} // namespace oms
#endif  // OMS_BASE_STREAM_FACTORY_H_
