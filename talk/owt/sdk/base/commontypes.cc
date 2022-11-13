// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"

namespace owt {
namespace base {
AudioEncodingParameters::AudioEncodingParameters() : codec(), max_bitrate(0) {}
AudioEncodingParameters::AudioEncodingParameters(
    const AudioCodecParameters& codec_param,
    unsigned long bitrate_bps)
    : codec(codec_param), max_bitrate(bitrate_bps) {}
AudioEncodingParameters::AudioEncodingParameters(
    const AudioEncodingParameters&) = default;
AudioEncodingParameters::~AudioEncodingParameters() = default;

VideoCodecParameters::VideoCodecParameters()
    : name(VideoCodec::kUnknown), profile("") {}
VideoCodecParameters::VideoCodecParameters(const VideoCodec& codec,
                                           const std::string& profile)
    : name(codec), profile(profile) {}
VideoCodecParameters::~VideoCodecParameters() = default;

VideoEncodingParameters::VideoEncodingParameters()
    : codec(), max_bitrate(0), hardware_accelerated(false) {}
VideoEncodingParameters::VideoEncodingParameters(
    const VideoCodecParameters& codec_param,
    unsigned long bitrate_bps,
    bool hw)
    : codec(codec_param), max_bitrate(bitrate_bps), hardware_accelerated(hw) {}
VideoEncodingParameters::~VideoEncodingParameters() = default;
}  // namespace base
}  // namespace owt