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

//
//MSDKVideoDecoderFactory implemenation
//

#include "talk/woogeen/sdk/base/win/mftvideodecoderfactory.h"
#include "talk/woogeen/sdk/base/win/mftmediadecoder.h"
#include "talk/woogeen/sdk/base/win/h264_msdk_decoder.h"
#include "talk/woogeen/sdk/base/win/h265_msdk_decoder.h"

MSDKVideoDecoderFactory::MSDKVideoDecoderFactory(HWND hWnd)
  :decoder_window_(hWnd){
    supported_codec_types_.clear();

    //Only BDW and SKL and above supports VP8 HW decoding. So we need to check if we're able to set input type on the
    //decoder mft. This is the simpliest method to check VP8 capability on IA.
    if (MSDKVideoDecoder::isVP8HWAccelerationSupported()) {
        supported_codec_types_.push_back(webrtc::kVideoCodecVP8);
    }

    bool is_h264_hw_supported = true;
    if (is_h264_hw_supported) {
        supported_codec_types_.push_back(webrtc::kVideoCodecH264);
    }
//TODO: add logic to detect plugin by MSDK.
    bool is_h265_hw_supported = true;
    if (is_h265_hw_supported) {
        supported_codec_types_.push_back(webrtc::kVideoCodecH265);
    }
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
            return new MSDKVideoDecoder(type, decoder_window_);
        } else if (*it == type && type == webrtc::kVideoCodecH264) {
            return new H264MSDKVideoDecoder(type, decoder_window_);
        } else if (*it == type && type == webrtc::kVideoCodecH265) {
            return new H265MSDKVideoDecoder(type, decoder_window_);
        }
    }
    return NULL;
}

void MSDKVideoDecoderFactory::DestroyVideoDecoder(
    webrtc::VideoDecoder* decoder) {
    delete decoder;
}