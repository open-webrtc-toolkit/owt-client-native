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

#ifndef ICS_BASE_MEDIA_CONSTRAINTS_H_
#define ICS_BASE_MEDIA_CONSTRAINTS_H_

#include "ics/base/commontypes.h"

namespace ics {
namespace base {

struct MediaStreamTrackAudioConstraints {
  double volume;
  unsigned long sample_rate;
  unsigned long channel_num;
};


struct MediaStreamTrackDeviceConstraintsForAudio : MediaStreamTrackAudioConstraints {
  std::string device_id;
};

struct MediaStreamTrackScreencastConstraintsForAudio : MediaStreamTrackAudioConstraints {
  int source_id;  // The handle of the app/desktop
};

struct MediaStreamTrackVideoConstraints {
  Resolution resolution;
  double frame_rate;
};

struct MediaStreamTrackDeviceConstraintsForVideo : MediaStreamTrackVideoConstraints {
  std::string device_id;
};

struct MediaStreamTrackScreencastConstraintsForVideo : MediaStreamTrackVideoConstraints {
  int source_id;  // The handle of the app/desktop
};

struct MediaStreamDeviceConstraints {
  MediaStreamTrackDeviceConstraintsForAudio audio_track_constraints;
  MediaStreamTrackDeviceConstraintsForVideo video_track_constraints;
};

} // namespace base
} // namespace ics

#endif  // ICS_BASE_MEDIA_CONSTRAINTS_H_
