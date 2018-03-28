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

#ifndef ICS_BASE_OPTIONS_H_
#define ICS_BASE_OPTIONS_H_

#include <vector>

#include "ics/base/commontypes.h"
#include "ics/base/mediaconstraints.h"

namespace ics {
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
} // namespace ics

#endif  // ICS_BASE_OPTIONS_H_
