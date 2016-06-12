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

#define MAX_VIDEO_WIDTH 3840
#define MAX_VIDEO_HEIGHT 2160
#define MAX_VIDEO_FPS 30

MSDKVideoEncoderFactory::MSDKVideoEncoderFactory(){
    supported_codecs_.clear();
    //Possibly enable this for KBL/CNL
    bool is_vp8_hw_supported = false;
    bool is_h264_hw_supported = true;
    // TODO(jianlin): find a way from MSDK to check h265 HW encoding support.
    // As we have SW, GAA & HW h265 encoding support, try loading plugins might be
    // a good way to determine that.
    bool is_h265_hw_supported = true;

    if (is_vp8_hw_supported) {
        supported_codecs_.push_back(VideoCodec(webrtc::kVideoCodecVP8, "VP8", MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT, MAX_VIDEO_FPS));
    }
    if (is_h264_hw_supported) {
        supported_codecs_.push_back(VideoCodec(webrtc::kVideoCodecH264, "H264", MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT, MAX_VIDEO_FPS));
    }
    if (is_h265_hw_supported) {
        supported_codecs_.push_back(VideoCodec(webrtc::kVideoCodecH265, "H265", MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT, MAX_VIDEO_FPS));
    }
}

MSDKVideoEncoderFactory::~MSDKVideoEncoderFactory(){}

webrtc::VideoEncoder* MSDKVideoEncoderFactory::CreateVideoEncoder(webrtc::VideoCodecType type){
    if (supported_codecs_.empty()){
        return NULL;
    }
    for (std::vector<VideoCodec>::const_iterator it = supported_codecs_.begin(); it != supported_codecs_.end(); ++it){
        if ((*it).type == type && type == webrtc::kVideoCodecH264) {
            return new H264VideoMFTEncoder();
        } else if ((*it).type == type && type == webrtc::kVideoCodecH265) {
            return new H265VideoMFTEncoder();
        }
    }
    return NULL;
}

const std::vector<MSDKVideoEncoderFactory::VideoCodec>& MSDKVideoEncoderFactory::codecs() const{
    return supported_codecs_;
}

void MSDKVideoEncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder* encoder){
    delete encoder;
}