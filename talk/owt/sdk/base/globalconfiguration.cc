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
#if defined(WEBRTC_WIN)
ID3D11Device* GlobalConfiguration::d3d11_decoding_device_ = nullptr;
#endif
bool GlobalConfiguration::flex_fec_enabled_ = false;
bool GlobalConfiguration::range_extension_enabled_ = false;
int GlobalConfiguration::link_mtu_ = 0; // not set;
int GlobalConfiguration::min_port_ = 0; // not set;
int GlobalConfiguration::max_port_ = 0; // not set;
bool GlobalConfiguration::low_latency_streaming_enabled_ = false;
bool GlobalConfiguration::log_latency_to_file_enabled_ = false;
bool GlobalConfiguration::encoded_frame_ = false;
int GlobalConfiguration::start_bitrate_kbps_ = 0; // not set
int GlobalConfiguration::min_bitrate_kbps_ = 0; // not set
int GlobalConfiguration::max_bitrate_kbps_ = 0; // not set

int GlobalConfiguration::delay_based_bwe_weight_ = 100;
std::unique_ptr<AudioFrameGeneratorInterface>
    GlobalConfiguration::audio_frame_generator_ = nullptr;
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
std::unique_ptr<VideoDecoderInterface>
    GlobalConfiguration::video_decoder_ = nullptr;
#endif
int GlobalConfiguration::h264_temporal_layers_ = 1;
#if defined(WEBRTC_IOS)
AudioProcessingSettings GlobalConfiguration::audio_processing_settings_ = {
    true, true, true, false};
#else
AudioProcessingSettings GlobalConfiguration::audio_processing_settings_ = {
    true, true, true, true};
#endif
bool GlobalConfiguration::pre_decode_dump_enabled_ = false;
bool GlobalConfiguration::post_encode_dump_enabled_ = false;
bool GlobalConfiguration::video_super_resolution_enabled_ = false;
}  // namespace base
}
