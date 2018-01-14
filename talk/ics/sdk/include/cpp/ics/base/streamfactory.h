/*
 * Copyright © 2017 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ICS_BASE_STREAM_FACTORY_H_
#define ICS_BASE_STREAM_FACTORY_H_

#include <unordered_map>
#inlcude "ics/base/stream.h"
#include "ics/base/exception.h"
#include "ics/base/localcamerastreamparameters.h"
#include "ics/base/macros.h"
#include "ics/base/videoencoderinterface.h"
#include "ics/base/videorendererinterface.h"

namespace webrtc {
  class MediaStreamInterface;
  class VideoTrackSourceInterface;
}

namespace ics {
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
} // namespace ics

#endif  // ICS_BASE_STREAM_FACTORY_H_
