// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_MEDIA_CONSTRAINTS_H_
#define OWT_BASE_MEDIA_CONSTRAINTS_H_
#include "owt/base/commontypes.h"
namespace owt {
namespace base {
struct OWT_EXPORT MediaStreamTrackAudioConstraints {
  double volume;
  unsigned long sample_rate;
  unsigned long channel_num;
};

struct OWT_EXPORT MediaStreamTrackDeviceConstraintsForAudio : MediaStreamTrackAudioConstraints {
  std::string device_id;
};
struct OWT_EXPORT MediaStreamTrackScreencastConstraintsForAudio : MediaStreamTrackAudioConstraints {
  int source_id;  // The handle of the app/desktop
};
struct OWT_EXPORT MediaStreamTrackVideoConstraints {
  Resolution resolution;
  double frame_rate;
};
struct OWT_EXPORT MediaStreamTrackDeviceConstraintsForVideo : MediaStreamTrackVideoConstraints {
  std::string device_id;
};
struct OWT_EXPORT MediaStreamTrackScreencastConstraintsForVideo : MediaStreamTrackVideoConstraints {
  int source_id;  // The handle of the app/desktop
};
struct OWT_EXPORT MediaStreamDeviceConstraints {
  MediaStreamTrackDeviceConstraintsForAudio audio_track_constraints;
  MediaStreamTrackDeviceConstraintsForVideo video_track_constraints;
};
} // namespace base
} // namespace owt
#endif  // OWT_BASE_MEDIA_CONSTRAINTS_H_
