// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/oms/sdk/base/customizedvideoencoderproxy.h"
#include "talk/oms/sdk/base/encodedvideoencoderfactory.h"
#include "webrtc/rtc_base/stringutils.h"
#include "webrtc/common_video/h264/profile_level_id.h"
#include "webrtc/common_types.h"
EncodedVideoEncoderFactory::EncodedVideoEncoderFactory() {
    supported_codecs_.clear();
    supported_codecs_.push_back(cricket::VideoCodec("VP8"));
    supported_codecs_.push_back(cricket::VideoCodec("VP9"));
#ifndef DISABLE_H265
    cricket::VideoCodec main10_high(cricket::kH265CodecName);
    main10_high.SetParam(cricket::kH265FmtpProfileSpace, "0");
    main10_high.SetParam(cricket::kH265FmtpProfileId, "1");
    main10_high.SetParam(cricket::kH265FmtpTierFlag, "0");
    main10_high.SetParam(cricket::kH265FmtpLevelId, "120");
    supported_codecs_.push_back(main10_high);
#endif
    const webrtc::H264::Level level = webrtc::H264::kLevel3_1;
     cricket::VideoCodec high(cricket::kH264CodecName);
     const webrtc::H264::ProfileLevelId high_profile(
          webrtc::H264::kProfileHigh, level);
     high.SetParam(
          cricket::kH264FmtpProfileLevelId,
          *webrtc::H264::ProfileLevelIdToString(high_profile));
     high.SetParam(cricket::kH264FmtpLevelAsymmetryAllowed, "1");
     high.SetParam(cricket::kH264FmtpPacketizationMode, "1");
     supported_codecs_.push_back(high);
    cricket::VideoCodec constrained_high(cricket::kH264CodecName);
    const webrtc::H264::ProfileLevelId constrained_high_profile(
        webrtc::H264::kProfileConstrainedHigh, level);
    constrained_high.SetParam(
        cricket::kH264FmtpProfileLevelId,
        *webrtc::H264::ProfileLevelIdToString(constrained_high_profile));
    constrained_high.SetParam(cricket::kH264FmtpLevelAsymmetryAllowed, "1");
    constrained_high.SetParam(cricket::kH264FmtpPacketizationMode, "1");
    supported_codecs_.push_back(constrained_high);
    cricket::VideoCodec constrained_baseline(cricket::kH264CodecName);
    const webrtc::H264::ProfileLevelId constrained_baseline_profile(
        webrtc::H264::kProfileConstrainedBaseline, level);
    constrained_baseline.SetParam(
        cricket::kH264FmtpProfileLevelId,
        *webrtc::H264::ProfileLevelIdToString(constrained_baseline_profile));
    constrained_baseline.SetParam(cricket::kH264FmtpLevelAsymmetryAllowed, "1");
    constrained_baseline.SetParam(cricket::kH264FmtpPacketizationMode, "1");
    supported_codecs_.push_back(constrained_baseline);
}
EncodedVideoEncoderFactory::~EncodedVideoEncoderFactory() {}
webrtc::VideoEncoder* EncodedVideoEncoderFactory::CreateVideoEncoder(
    const cricket::VideoCodec& codec) {
  if (supported_codecs_.empty()) {
    return nullptr;
  }
  webrtc::VideoCodecType codec_type;
  if (FindMatchingCodec(supported_codecs_, codec)) {
    if (!_stricmp(codec.name.c_str(), "H264"))
      codec_type = webrtc::kVideoCodecH264;
    else if(!_stricmp(codec.name.c_str(), "VP8"))
      codec_type = webrtc::kVideoCodecVP8;
    else if (!_stricmp(codec.name.c_str(), "VP9"))
        codec_type = webrtc::kVideoCodecVP9;
#ifndef DISABLE_H265
    else if (!_stricmp(codec.name.c_str(), "H265"))
        codec_type = webrtc::kVideoCodecH265;
#endif
    else {
      return nullptr;
    }
    return new oms::base::CustomizedVideoEncoderProxy(codec_type);
  }
  return nullptr;
}
const std::vector<cricket::VideoCodec>&
EncodedVideoEncoderFactory::supported_codecs() const {
  return supported_codecs_;
}
void EncodedVideoEncoderFactory::DestroyVideoEncoder(
    webrtc::VideoEncoder* encoder) {
  delete encoder;
}
