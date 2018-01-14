/*
* Intel License
*/
#include "talk/ics/sdk/base/win/mftvideodecoderfactory.h"
#include "talk/ics/sdk/base/win/mftmediadecoder.h"
#include "talk/ics/sdk/base/win/external_msdk_decoder.h"
#include "talk/ics/sdk/base/win/h265_msdk_decoder.h"

MSDKVideoDecoderFactory::MSDKVideoDecoderFactory() {
  supported_codec_types_.clear();

  // Only BDW and SKL and above supports VP8 HW decoding. So we need to check if
  // we're able to set input type on the
  // decoder mft. This is the simpliest method to check VP8 capability on IA.
  if (MSDKVideoDecoder::isVP8HWAccelerationSupported()) {
    supported_codec_types_.push_back(webrtc::kVideoCodecVP8);
  }

  bool is_h264_hw_supported = true;
  if (is_h264_hw_supported) {
    supported_codec_types_.push_back(webrtc::kVideoCodecH264);
  }
#ifndef DISABLE_H265
//TODO: add logic to detect plugin by MSDK.
    bool is_h265_hw_supported = true;
    if (is_h265_hw_supported) {
        supported_codec_types_.push_back(webrtc::kVideoCodecH265);
    }
#endif
}


MSDKVideoDecoderFactory::~MSDKVideoDecoderFactory() {
    MFShutdown();
    ::CoUninitialize();
}

webrtc::VideoDecoder* MSDKVideoDecoderFactory::CreateVideoDecoder(webrtc::VideoCodecType type) {
    if (supported_codec_types_.empty()) {
        return NULL;
    }
    for (std::vector<webrtc::VideoCodecType>::const_iterator it =
        supported_codec_types_.begin(); it != supported_codec_types_.end();
        ++it) {
        if (*it == type && type == webrtc::kVideoCodecVP8) {
          return new ics::base::ExternalMSDKVideoDecoder(type);
        } else if (*it == type && type == webrtc::kVideoCodecH264) {
          return new ics::base::ExternalMSDKVideoDecoder(type);
#ifndef DISABLE_H265
        } else if (*it == type && type == webrtc::kVideoCodecH265) {
          return new H265MSDKVideoDecoder(type);
#endif
        }
    }
    return NULL;
}

void MSDKVideoDecoderFactory::DestroyVideoDecoder(
    webrtc::VideoDecoder* decoder) {
    delete decoder;
}
