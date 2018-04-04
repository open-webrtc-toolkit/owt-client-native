/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
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

#ifndef ICS_CONFERENCE_SUBSCRIBEOPTIONS_H_
#define ICS_CONFERENCE_SUBSCRIBEOPTIONS_H_

#include "ics/base/commontypes.h"

namespace ics {
namespace conference {

/// Audio subscription contraints.
struct AudioSubscriptionConstraints {
  std::vector<ics::base::AudioCodecParameters> codecs;
};

/// Video subscription constraints.
struct VideoSubscriptionConstraints {
  /**
   @brief Construct VideoSubscriptionConstraints with default values.
   @defatils By default the publication settings of stream is used.
  */
    explicit VideoSubscriptionConstraints()
        : resolution(0, 0)
        , frameRate(0)
        , bitrateMultiplier(0)
        , keyFrameInterval(0) {}

  std::vector<ics::base::VideoCodecParameters> codecs;
  ics::base::Resolution resolution;
  double frameRate;
  double bitrateMultiplier;
  unsigned long keyFrameInterval;
};

/// Subscribe options
struct SubscribeOptions {
  AudioSubscriptionConstraints audio;
  VideoSubscriptionConstraints video;
};

/// Video subscription update constrains used by subscription's ApplyOptions API.
struct VideoSubscriptionUpdateConstraints {
  /**
   @brief Construct VideoSubscriptionUpdateConstraints with default value.
   */
  explicit VideoSubscriptionUpdateConstraints()
      : resolution(0, 0),
        frameRate(0),
        bitrateMultiplier(0),
        keyFrameInterval(0) {}

  ics::base::Resolution resolution;
  double frameRate;
  double bitrateMultiplier;
  unsigned long keyFrameInterval;
};

/// Subscription update option used by subscription's ApplyOptions API.
struct SubscriptionUpdateOptions {
  /// Options for updating a subscription.
  VideoSubscriptionUpdateConstraints video;
};
}
}

#endif  // ICS_CONFERENCE_SUBSCRIBEOPTIONS_H_
