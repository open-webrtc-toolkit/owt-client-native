// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OPTIONS_H_
#define OWT_BASE_OPTIONS_H_

#include <string>
#include <vector>
#include "owt/base/commontypes.h"
#include "owt/base/mediaconstraints.h"
namespace owt {
namespace base {
/// Audio subscription capabilities. Empty means not setting corresponding
/// capability.
struct OWT_EXPORT AudioSubscriptionCapabilities {
  std::vector<AudioCodecParameters> codecs;
};

/// Video subscription capabilities. Empty means not setting corresponding
/// capability.
struct OWT_EXPORT VideoSubscriptionCapabilities {
  std::vector<VideoCodecParameters> codecs;
  std::vector<Resolution> resolutions;
  std::vector<double> frame_rates;
  std::vector<double> bitrate_multipliers;
  std::vector<unsigned long> keyframe_intervals;
};
struct OWT_EXPORT SubscriptionCapabilities {
  AudioSubscriptionCapabilities audio;
  VideoSubscriptionCapabilities video;
};
struct OWT_EXPORT AudioPublicationSettings {
  AudioCodecParameters codec;
};
struct OWT_EXPORT VideoPublicationSettings {
  VideoCodecParameters codec;
  Resolution resolution;
  double frame_rate;
  unsigned long bitrate;
  unsigned long keyframe_interval;
  std::string rid;
  std::string track_id;
};

#ifdef OWT_ENABLE_QUIC
struct OWT_EXPORT TransportSettings {
  explicit TransportSettings() : transport_type(TransportType::kWebRTC) {}
  TransportType transport_type;
};
#endif

struct OWT_EXPORT PublicationSettings {
  std::vector<AudioPublicationSettings> audio;
  std::vector<VideoPublicationSettings> video;
#ifdef OWT_ENABLE_QUIC
  TransportSettings transport;
#endif
};
/**
 @brief Publish options describing encoding settings.
 @details Set encoding constraint on video or video using this option.
*/
struct OWT_EXPORT PublishOptions {
  std::vector<AudioEncodingParameters> audio;
  std::vector<VideoEncodingParameters> video;
#ifdef OWT_ENABLE_QUIC
  TransportConstraints transport;
#endif
};

}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_OPTIONS_H_
