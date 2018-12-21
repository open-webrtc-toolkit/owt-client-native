// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/oms/sdk/base/win/msdkvideoencoder.h"
#include "talk/oms/sdk/base/win/msdkvideoencoderfactory.h"
#include "webrtc/common_video/h264/profile_level_id.h"

namespace oms {
namespace base {

MSDKVideoEncoderFactory::MSDKVideoEncoderFactory(){
    supported_codecs_.clear();
  // We will not turn on VP8 encoding HW support at present.
  bool is_vp8_hw_supported = false;
  bool is_h264_hw_supported = true;
#ifndef DISABLE_H265
  // TODO(jianlin): Find a way from MSDK to check h265 HW encoding support.
  // As we have SW, GAA & HW h265 encoding support, try loading plugins might be
  // a good way to determine that.
  bool is_h265_hw_supported = true;
#endif

  if (is_vp8_hw_supported) {
    supported_codecs_.push_back(cricket::VideoCodec("VP8"));
  }
  if (is_h264_hw_supported) {
    supported_codecs_.push_back(cricket::VideoCodec("H264"));
  }
  const webrtc::H264::Level level = webrtc::H264::kLevel3_1;

  cricket::VideoCodec baseline(cricket::kH264CodecName);
  const webrtc::H264::ProfileLevelId baseline_profile(
        webrtc::H264::kProfileBaseline, level);
  baseline.SetParam(
       cricket::kH264FmtpProfileLevelId,
       *webrtc::H264::ProfileLevelIdToString(baseline_profile));
  baseline.SetParam(cricket::kH264FmtpLevelAsymmetryAllowed, "1");
  baseline.SetParam(cricket::kH264FmtpPacketizationMode, "1");
  supported_codecs_.push_back(baseline);

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

#ifndef DISABLE_H265
  if (is_h265_hw_supported) {
    cricket::VideoCodec main10_high(cricket::kH265CodecName);
    main10_high.SetParam(cricket::kH265FmtpProfileSpace, "0");
    main10_high.SetParam(cricket::kH265FmtpProfileId, "1");
    main10_high.SetParam(cricket::kH265FmtpTierFlag, "0");
    main10_high.SetParam(cricket::kH265FmtpLevelId, "120");
    supported_codecs_.push_back(main10_high);
  }
#endif
}

MSDKVideoEncoderFactory::~MSDKVideoEncoderFactory(){}

webrtc::VideoEncoder* MSDKVideoEncoderFactory::CreateVideoEncoder(
    const cricket::VideoCodec& codec) {
  if (supported_codecs_.empty()) {
    return nullptr;
  }

  if (FindMatchingCodec(supported_codecs_, codec) &&
      (!_stricmp(codec.name.c_str(), "H264") ||!_stricmp(codec.name.c_str(), "H265")) ) {
    return new MSDKVideoEncoder();
  }

  return nullptr;
}

const std::vector<cricket::VideoCodec>&
MSDKVideoEncoderFactory::supported_codecs() const {
  return supported_codecs_;
}

void MSDKVideoEncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder* encoder){
  delete encoder;
}
}  // namespace base
}  // namespace oms
