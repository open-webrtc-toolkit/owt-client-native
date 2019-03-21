// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OPTIONS_H_
#define OWT_BASE_OPTIONS_H_
#include <vector>
#include "owt/base/commontypes.h"
#include "owt/base/mediaconstraints.h"
namespace owt {
namespace base {
/// Audio subscription capabilities. Empty means not setting corresponding capability.
struct AudioSubscriptionCapabilities {
  std::vector<AudioCodecParameters>    codecs;
};

/// Video subscription capabilities. Empty means not setting corresponding capability.
struct VideoSubscriptionCapabilities {
  std::vector<VideoCodecParameters>    codecs;
  std::vector<Resolution>              resolutions;
  std::vector<double>                  frame_rates;
  std::vector<double>                  bitrate_multipliers;
  std::vector<unsigned long>           keyframe_intervals;
};
struct SubscriptionCapabilities {
  AudioSubscriptionCapabilities  audio;
  VideoSubscriptionCapabilities  video;
};
struct AudioPublicationSettings {
  AudioCodecParameters     codec;
};
struct VideoPublicationSettings {
  VideoCodecParameters     codec;
  Resolution               resolution;
  double                   frame_rate;
  unsigned long            bitrate;
  unsigned long            keyframe_interval;
};
struct PublicationSettings {
  AudioPublicationSettings  audio;
  VideoPublicationSettings  video;
};
/**
 @brief Publish options describing encoding settings.
 @details Set encoding constraint on video or video using this option.
*/
struct PublishOptions {
  std::vector<AudioEncodingParameters>  audio;
  std::vector<VideoEncodingParameters>  video;
};

} // namespace base
} // namespace owt
#endif  // OWT_BASE_OPTIONS_H_
