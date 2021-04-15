// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "owt/base/globalconfiguration.h"
namespace owt {
namespace base {
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
// Enable hardware acceleration by default is on.
bool GlobalConfiguration::hardware_acceleration_enabled_ = true;
#endif
bool GlobalConfiguration::encoded_frame_ = false;
std::unique_ptr<AudioFrameGeneratorInterface>
    GlobalConfiguration::audio_frame_generator_ = nullptr;
std::unique_ptr<VideoDecoderInterface>
    GlobalConfiguration::video_decoder_ = nullptr;
int GlobalConfiguration::h264_temporal_layers_ = 1;
#if defined(WEBRTC_IOS)
AudioProcessingSettings GlobalConfiguration::audio_processing_settings_ = {
    true, true, true, false};
#else
AudioProcessingSettings GlobalConfiguration::audio_processing_settings_ = {
    true, true, true, true};
#endif
int GlobalConfiguration::audio_min_ = 0;
int GlobalConfiguration::audio_max_ = 0;
int GlobalConfiguration::video_min_ = 0;
int GlobalConfiguration::video_max_ = 0;
int GlobalConfiguration::screen_min_ = 0;
int GlobalConfiguration::screen_max_ = 0;
int GlobalConfiguration::data_min_ = 0;
int GlobalConfiguration::data_max_ = 0;
bool GlobalConfiguration::pre_decode_dump_enabled_ = false;
}  // namespace base
}
