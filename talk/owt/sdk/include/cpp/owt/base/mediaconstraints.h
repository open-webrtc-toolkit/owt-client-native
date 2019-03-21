// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_MEDIA_CONSTRAINTS_H_
#define OWT_BASE_MEDIA_CONSTRAINTS_H_
#include "owt/base/commontypes.h"
namespace owt {
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
} // namespace owt
#endif  // OWT_BASE_MEDIA_CONSTRAINTS_H_
