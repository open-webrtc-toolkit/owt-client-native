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

#ifndef ICS_BASE_COMMONTYPES_H_
#define ICS_BASE_COMMONTYPES_H_

#include <string>
#include <vector>
#include <unordered_map>

namespace ics {
namespace base {

/// Audio codec
enum class AudioCodec : int {
  kPcmu = 1,   ///< g711 u-law
  kPcma,   ///< g711 a-law
  kOpus,   ///< opus
  kG722,   ///< g722
  kIsac,   ///< iSAC
  kIlbc,   ///< iLBC
  kAac,    ///< AAC or HE-AAC
  kAc3,    ///< AC3
  kAsao,   ///< Nellymoser
  kUnknown
};

/// Video codec
enum class VideoCodec : int {
  kVp8 = 1,
  kVp9,
  kH264,
  kH265,
  kUnknown
};

/// Track kind
enum class TrackKind : int{
  kAudio = 1,
  kVideo,
  kAudioAndVideo,
  kUnknown
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

/// Audio codec parameters for an audio track.
struct AudioCodecParameters {
  /// Construct an instance of AudioCodecParameters with default param.
  AudioCodecParameters()
      : name(AudioCodec::kUnknown), channel_count(0), clock_rate(0) {}
  /// Construct an instance of AudioCodecParameters with codec name/channel count and clock rate.
  AudioCodecParameters(const AudioCodec& codec_name,
                       unsigned long channel_count,
                       unsigned long clock_rate)
      : name(codec_name),
        channel_count(channel_count),
        clock_rate(clock_rate) {}

  AudioCodec name;
  unsigned long channel_count;
  unsigned long clock_rate;
};

/// Audio encoding parameters.
struct AudioEncodingParameters {
  explicit AudioEncodingParameters() : codec(), max_bitrate(0) {}

  AudioEncodingParameters(const AudioCodecParameters& codec_param,
                          unsigned long bitrate_bps)
      : codec(codec_param), max_bitrate(bitrate_bps) {}

  AudioEncodingParameters(const AudioEncodingParameters& aep) = default;

  AudioEncodingParameters& operator=(const AudioEncodingParameters&) = default;

  AudioCodecParameters codec;
  unsigned long max_bitrate;
};

/// Video codec parameters for a video track.
struct VideoCodecParameters {
  /// Construct an instance of VideoCodecParameters with default parameters.
  explicit VideoCodecParameters() : name(VideoCodec::kUnknown), profile("") {}
  /// Construct an instance of VideoCodecParameter with codec name and profile.
  VideoCodecParameters(const VideoCodec& codec, const std::string& profile)
      : name(codec), profile(profile) {}

  VideoCodec name;
  std::string profile;
};

/// Video encoding parameters. Used to specify the video encoding settings when
/// publishing the video.
struct VideoEncodingParameters {
  explicit VideoEncodingParameters()
      : codec(), max_bitrate(0), hardware_accelerated(false) {}
  /// Construct an instance of VideoEncodingParameters
  VideoEncodingParameters(const VideoCodecParameters& codec_param,
                          unsigned long bitrate_bps,
                          bool hw)
      : codec(codec_param),
        max_bitrate(bitrate_bps),
        hardware_accelerated(hw) {}

  VideoEncodingParameters(const VideoEncodingParameters& aep) = default;

  VideoEncodingParameters& operator=(const VideoEncodingParameters&) = default;

  VideoCodecParameters codec;
  unsigned long max_bitrate;
  bool hardware_accelerated;
};

/// Audio source info.
///
/// This enumeration defines possible audio sources
enum class AudioSourceInfo : int {
  kMic = 1,     ///< Microphone
  kScreenCast,  ///< Screen-cast
  kFile,        ///< From file
  kMixed,       ///< From MCU mix engine
  kUnknown
};

/// Video source info.
///
/// This enumeration defines possible video sources.
enum class VideoSourceInfo : int {
  kCamera = 1,  ///< Camera
  kScreenCast,  ///< Screen-cast
  kFile,        ///< From file
  kMixed,       ///< From MCU mix engine
  kUnknown
};

/// Stream source.
struct StreamSourceInfo {
  explicit StreamSourceInfo()
    : audio(AudioSourceInfo::kUnknown),
      video(VideoSourceInfo::kUnknown) {}

  StreamSourceInfo(AudioSourceInfo audio_source, VideoSourceInfo video_source)
    : audio(audio_source),
      video(video_source) {}
  /// The audio source info of the stream
  AudioSourceInfo audio;
  /// The video source info of the stream
  VideoSourceInfo video;
};

}  // namespace base
}  // namespace ics

#endif  // ICS_BASE_COMMONTYPES_H_
