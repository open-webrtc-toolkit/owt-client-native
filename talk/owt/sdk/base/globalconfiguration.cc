// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "owt/base/globalconfiguration.h"
namespace owt {
namespace base {
#if defined(WEBRTC_WIN)
// Enable hardware acceleration by default is on.
bool GlobalConfiguration::hardware_acceleration_enabled_ = true;
#endif
#if defined(OWT_CUSTOM_AVIO)
bool GlobalConfiguration::encoded_frame_ = false;
std::unique_ptr<AudioFrameGeneratorInterface>
    GlobalConfiguration::audio_frame_generator_ = nullptr;
std::unique_ptr<VideoDecoderInterface>
    GlobalConfiguration::video_decoder_ = nullptr;
#endif
#if defined(WEBRTC_IOS)
AudioProcessingSettings GlobalConfiguration::audio_processing_settings_ = {
    true, true, true, false};
#else
AudioProcessingSettings GlobalConfiguration::audio_processing_settings_ = {
    true, true, true, true};
#endif
}
}
