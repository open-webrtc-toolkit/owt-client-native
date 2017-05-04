/*
 * Intel License
 */
#include "talk/woogeen/sdk/base/customizedvideoencoderproxy.h"
#include "talk/woogeen/sdk/base/encodedvideoencoderfactory.h"
#include "webrtc/base/stringutils.h"
#include "webrtc/common_video/h264/profile_level_id.h"
#include "webrtc/common_types.h"

EncodedVideoEncoderFactory::EncodedVideoEncoderFactory() {
    supported_codecs_.clear();
    supported_codecs_.push_back(cricket::VideoCodec("VP8"));
    supported_codecs_.push_back(cricket::VideoCodec("VP9"));
#ifndef DISABLE_H265
    supported_codecs_.push_back(cricket::VideoCodec("H265"));
#endif
    const webrtc::H264::Level level = webrtc::H264::kLevel3_1;

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
    return new woogeen::base::CustomizedVideoEncoderProxy(codec_type);
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
