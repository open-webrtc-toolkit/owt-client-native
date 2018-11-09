/*
* Intel License
*/
#include "talk/oms/sdk/base/win/mftvideoencoderfactory.h"
#include "talk/oms/sdk/base/win/h264_video_mft_encoder.h"
#include "talk/oms/sdk/base/win/h265_msdk_encoder.h"
#include "webrtc/common_video/h264/profile_level_id.h"
#define MAX_VIDEO_WIDTH 3840
#define MAX_VIDEO_HEIGHT 2160
#define MAX_VIDEO_FPS 30
MSDKVideoEncoderFactory::MSDKVideoEncoderFactory(){
    supported_codecs_.clear();
    //Possibly enable this for KBL/CNL
    bool is_vp8_hw_supported = false;
    bool is_h264_hw_supported = true;
#ifndef DISABLE_H265
    // TODO(jianlin): find a way from MSDK to check h265 HW encoding support.
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
    return NULL;
  }
  if (FindMatchingCodec(supported_codecs_, codec) &&
      !_stricmp(codec.name.c_str(), "H264")) {
    return new H264VideoMFTEncoder();
#ifndef DISABLE_H265
  } else if (FindMatchingCodec(supported_codecs_, codec) &&
             !_stricmp(codec.name.c_str(), "H265")) {
    return new H265VideoMFTEncoder();
#endif
  }
  return NULL;
}
const std::vector<cricket::VideoCodec>&
MSDKVideoEncoderFactory::supported_codecs() const {
  return supported_codecs_;
}
void MSDKVideoEncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder* encoder){
    delete encoder;
}
