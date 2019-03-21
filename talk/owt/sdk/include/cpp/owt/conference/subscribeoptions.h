// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_SUBSCRIBEOPTIONS_H_
#define OWT_CONFERENCE_SUBSCRIBEOPTIONS_H_
#include "owt/base/commontypes.h"
namespace owt {
namespace conference {
/// Audio subscription contraints.
struct AudioSubscriptionConstraints {
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
struct VideoSubscriptionConstraints {
  /**
   @brief Construct VideoSubscriptionConstraints with default values.
   @details By default the publication settings of stream is used.
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
/// Subscribe options
struct SubscribeOptions {
  AudioSubscriptionConstraints audio;
  VideoSubscriptionConstraints video;
};
/// Video subscription update constrains used by subscription's ApplyOptions
/// API.
struct VideoSubscriptionUpdateConstraints {
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
/// Subscription update option used by subscription's ApplyOptions API.
struct SubscriptionUpdateOptions {
  /// Options for updating a subscription.
  VideoSubscriptionUpdateConstraints video;
};
}  // namespace conference
}  // namespace owt
#endif  // OWT_CONFERENCE_SUBSCRIBEOPTIONS_H_
