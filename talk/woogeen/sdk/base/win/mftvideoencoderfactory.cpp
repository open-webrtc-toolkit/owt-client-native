/*
* libjingle
* Copyright 2012 Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
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

#include "talk/woogeen/sdk/base/win/mftvideoencoderfactory.h"
#include "talk/woogeen/sdk/base/win/h264_video_mft_encoder.h"
#include "talk/woogeen/sdk/base/win/h265_msdk_encoder.h"
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
      supported_codecs_.push_back(cricket::VideoCodec("H265"));
    }
#endif
}

MSDKVideoEncoderFactory::~MSDKVideoEncoderFactory(){}

webrtc::VideoEncoder* MSDKVideoEncoderFactory::CreateVideoEncoder(
    const cricket::VideoCodec& codec) {
  if (supported_codecs_.empty()) {
    return NULL;
  }
  for (std::vector<cricket::VideoCodec>::const_iterator it =
           supported_codecs_.begin();
       it != supported_codecs_.end(); ++it) {
    if (FindMatchingCodec(supported_codecs_, codec) &&
        !_stricmp(codec.name.c_str(), "H264")) {
      return new H264VideoMFTEncoder();
#ifndef DISABLE_H265
    } else if (FindMatchingCodec(supported_codecs_, codec) &&
               !_stricmp(codec.name.c_str(), "H265")) {
      return new H265VideoMFTEncoder();
#endif
    }
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
