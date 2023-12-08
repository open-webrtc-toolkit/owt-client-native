// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_CONFERENCE_SUBSCRIBEOPTIONS_H_
#define OWT_CONFERENCE_SUBSCRIBEOPTIONS_H_
#include "owt/base/commontypes.h"
namespace owt {
namespace conference {
/// Audio subscription contraints.
struct OWT_EXPORT AudioSubscriptionConstraints {
  /**
   @brief Construct AudioSubcriptionConstraints with defaut settings.
   @details By default the audio suscription is enabled.
  */
  explicit AudioSubscriptionConstraints() :
      disabled(false) {}
  bool disabled;
  std::vector<owt::base::AudioCodecParameters> codecs;
};
/// Video subscription constraints.
struct OWT_EXPORT VideoSubscriptionConstraints {
  /**
   @brief Construct VideoSubscriptionConstraints with default values.
   @details By default the publication settings of stream is used.
   if rid is specified, other fields will be ignored.
  */
  explicit VideoSubscriptionConstraints()
      : disabled(false),
        resolution(0, 0),
        frameRate(0),
        bitrateMultiplier(0),
        keyFrameInterval(0) {}
  bool disabled;
  std::vector<owt::base::VideoCodecParameters> codecs;
  owt::base::Resolution resolution;
  double frameRate;
  double bitrateMultiplier;
  unsigned long keyFrameInterval;
};
/// Simulcast and SVC stream subscription constranits.
struct VideoSubscriptionConstraints2 {
  explicit VideoSubscriptionConstraints2()
      : rid(""), spatialLayerId(-1), temporalLayerId(-1) {}
  std::string rid;
  int spatialLayerId;
  int temporalLayerId;
};

#ifdef OWT_ENABLE_QUIC
/// Data subscription constraints.
struct OWT_EXPORT DataSubscriptionConstraints {
  explicit DataSubscriptionConstraints() : enabled(false) {}
  bool enabled;
};
#endif

/// Subscribe options
struct OWT_EXPORT SubscribeOptions {
  AudioSubscriptionConstraints audio;
  VideoSubscriptionConstraints video;
#ifdef OWT_ENABLE_QUIC
  DataSubscriptionConstraints data;
#endif
};

/// SVC and Simulcast stream subscribe options
struct SubscribeOptions2 {
  AudioSubscriptionConstraints audio;
  VideoSubscriptionConstraints2 video;
};
/// Video subscription update constrains used by subscription's ApplyOptions
/// API.
struct OWT_EXPORT VideoSubscriptionUpdateConstraints {
  /**
   @brief Construct VideoSubscriptionUpdateConstraints with default value.
   */
  explicit VideoSubscriptionUpdateConstraints()
      : resolution(0, 0),
        frameRate(0),
        bitrateMultiplier(0),
        keyFrameInterval(0) {}
  owt::base::Resolution resolution;
  double frameRate;
  double bitrateMultiplier;
  unsigned long keyFrameInterval;
};
/// Simulcast and SVC stream subscription update constraints
struct VideoSubscriptionUpdateConstraints2 {
  /**
   @brief Construct VideoSubscriptionUpdateConstraints with default value.
   */
  explicit VideoSubscriptionUpdateConstraints2()
      : rid(""), spatialLayerId(-1), temporalLayerId(-1) {}
  std::string rid;
  int spatialLayerId;
  int temporalLayerId;
};
/// Subscription update option used by subscription's ApplyOptions API.
struct OWT_EXPORT SubscriptionUpdateOptions {
  /// Options for updating a subscription.
  VideoSubscriptionUpdateConstraints video;
};
/// Simulcast and SVC stream subcription update option used by subscription's
/// ApplyOptions API.
struct SubscriptionUpdateOptions2 {
  /// Options for updating a subscription.
  VideoSubscriptionUpdateConstraints2 video;
};
}  // namespace conference
}  // namespace owt
#endif  // OWT_CONFERENCE_SUBSCRIBEOPTIONS_H_
