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

#ifndef ICS_BASE_COMMON_TYPES_H_
#define ICS_BASE_COMMON_TYPES_H_

#include <vector>
#include <unordered_map>

namespace ics {
namespace base {

/// Audio codec
enum AudioCodec {
  kPCMU,   ///< g711 u-law
  kPCMA,   ///< g711 a-law
  kOPUS,   ///< opus
  kG722,   ///< g722
  kISAC,   ///< iSAC
  kILBC,   ///< iLBC
  kAAC,    ///< AAC or HE-AAC
  kAC3,    ///< AC3
  kASAO,   ///< Nellymoser
  kAudioCodecUnknown
};

/// Video codec
enum VideoCodec {
  kVP8,
  kVP9,
  kH264,
  kH265,
  kVideoCodecUnknown
};

/// Track kind
enum TrackKind {
  kTrackAudio,
  kTrackVideo,
  kTrackAudioVideo,
  kTrackUnknown
};

/// This class represents a resolution value.
struct Resolution {
  /// Construct an instance with width and height equal to 0.
  explicit Resolution(): width(0), height(0) {}
  /// Construct an instance with specify width and height.
  Resolution(unsigned long w, unsigned long h) : width(w), height(h) {}

  bool operator==(const Resolution& rhs) const {
    return this->width == rhs.width && this->height == rhs.height;
  }

  unsigned long width;
  unsigned long height;
};

/**
 * @brief An instance of this class represent a video format.
 */
struct VideoFormat {
  /** @cond */
  explicit VideoFormat(const Resolution& r): resolution(r){}
  /** @endcond */
  Resolution resolution;
};

/// Audio codec parameters for an audio track.
struct AudioCodecParameters {
  // Construct an instance of AudioCodecParameters
  AudioCodecParameters() : codec_name_(""), channel_count_(0), clock_rate_(0) {}

  AudioCodecParameters(const std::string& codec_name, unsigned long channel_count, unsigned long clock_rate)
    : codec_name_(codec_name),
      channel_count_(channel_count),
      clock_rate_(clock_rate) {}

  std::string codec_name_;
  unsigned long channel_count_;
  unsigned long clock_rate_;
};

/// Audio encoding parameters.
struct AudioEncodingParameters {
   explicit AudioEncodingParameters() : codec_params(), maxBitrateBps(0) {}

   AudioEncodingParameters (AudioCodecParameters codec_param, unsigned long bitrate_bps)
     : codec_params(codec_param),
       maxBitrateBps(bitrate_bps) {}

   AudioEncodingParameters (const AudioEncodingParameters& aep) = default;

   AudioEncodingParameters& operator=(const AudioEncodingParameters&) = default;

   AudioCodecParameters codec_params;
   unsigned long maxBitrateBps;
};

/// Video codec parameters for a video track.
struct VideoCodecParameters {
  // Construct an instance of VideoCodecParameters
  explicit VideoCodecParameters() : codec_name_(""), profile_("") {}

  VideoCodecParameters (const std::string& codec, const std::string& profile) : codec_name_(codec), profile_(profile) {}

  std::string codec_name_;
  std::string profile_;
};

/// Video encoding parameters.
struct VideoEncodingParameters {
   explicit VideoEncodingParameters() : codec_params(), maxBitrateBps(0), hardware_accelerated(false) {}

   VideoEncodingParameters (VideoCodecParameters codec_param, unsigned long bitrate_bps, bool hw)
     : codec_params(codec_param),
       maxBitrateBps(bitrate_bps),
       hardware_accelerated(hw) {}

   VideoEncodingParameters (const VideoEncodingParameters& aep) = default;

   VideoEncodingParameters& operator=(const VideoEncodingParameters&) = default;

   VideoCodecParameters codec_params;
   unsigned long maxBitrateBps;
   bool hardware_accelerated;
};

/// Audio source info.
///
/// This enumeration defines possible audio sources
enum AudioSourceInfo {
  kAudioMIC,               ///< Microphone
  kAudioScreenCast,        ///< Screen-cast
  kAudioFile,              ///< From file
  kAudioSourceUnknown
};

/// Video source info.
///
/// This enumeration defines possible video sources.
enum VideoSourceInfo {
  kVideoCamera,           ///< Camera
  kVideoScreenCast,       ///< Screen-cast
  kVideoFile,             ///< From file
  kVideoSourceUnknown
};

/// Stream source.
struct StreamSourceInfo {
  AudioSourceInfo audio_source;
  VideoSourceInfo video_source;
};

}  // namespace base
}  // namespace ics

#endif  // ICS_BASE_COMMON_TYPES_H_
