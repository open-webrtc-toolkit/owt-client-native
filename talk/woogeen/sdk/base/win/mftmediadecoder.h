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
#include <utility>
#include <vector>

#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "webrtc/base/common.h"
//#include "webrtc/base/json.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/bind.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/thread.h"

//For decoder and encoder factory
#include "talk/media/webrtc/webrtcvideodecoderfactory.h"
#include "talk/media/webrtc/webrtcvideoencoderfactory.h"
#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/common_video/interface/i420_buffer_pool.h"

#include "third_party/libyuv/include/libyuv.h"

#include "talk/woogeen/sdk/base/win/d3dnativeframe.h"
#include <atlbase.h>
#include <codecapi.h>
//#include <ks.h>
#include <combaseapi.h>
#include <wmcodecdsp.h>
#include <mfapi.h>
#include <mferror.h>
#include <mftransform.h>
#include <mfidl.h>
#include <d3d9.h>
#include <dxva2api.h>
#pragma once

enum Version{
    VERSION_XP = 0,
    VERSION_VISTA,
    VERSION_WIN7,
    VERSION_WIN8,
    VERSION_WIN8_1,
    VERSION_WIN10,
    VERSION_WIN_LAST,
};

enum WOW64Status {
    WOW64_DISABLED,
    WOW64_ENABLED,
    WOW64_UNKNOWN,
};

struct VersionNumber{
    int major;
    int minor;
    int build;
};

//
//MSDK Video Decoder declaration.
//

class MSDKVideoDecoder : public webrtc::VideoDecoder,
    public rtc::MessageHandler {
public:
    enum State{
        kUnitialized,
        kNormal,
        kResetting,
        kStopped,
        kFlushing,
    };
    MSDKVideoDecoder(webrtc::VideoCodecType codecType, HWND decoder_window);
    virtual ~MSDKVideoDecoder();

    int32_t InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores)
        override;

    int32_t Decode(
        const webrtc::EncodedImage& inputImage, bool missingFrames,
        const webrtc::RTPFragmentationHeader* fragmentation,
        const webrtc::CodecSpecificInfo* codecSpecificInfo = NULL,
        int64_t renderTimeMs = -1) override;

    int32_t RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
        override;

    int32_t Release() override;

    int32_t Reset() override;
    // rtc::MessageHandler implementation.
    void OnMessage(rtc::Message* msg) override;

    static bool isVP8HWAccelerationSupported();
private:
    //Utility methods to decide which MSDK dlls to be loaded.
    Version GetOSVersion();
    WOW64Status GetWOW64Status();

    int32_t InitDecodeOnCodecThread();
    int32_t DecodeOnCodecThread(const webrtc::EncodedImage& inputImage);
    int32_t DoDecode();
    int32_t DecodeInternal(IMFSample* sample);
    void CheckOnCodecThread();

    bool SetDecoderMediaTypes();
    bool SetDecoderInputMediaType();
    bool SetDecoderOutputMediaType(const GUID& subtype);

    bool OutputSamplesPresent();

    int32_t ProcessOutputSample(IMFSample* sample);
    void DecodePendingInputBuffers();

    bool GetStreamsInfoAndBufferReqs();

    bool CreateD3DDeviceManager();

    void SetState(State state);

    State GetDecoderState();
    DWORD GetGraphicsVertexCaps();
    // Type of video codec.
    webrtc::VideoCodecType codecType_;

    rtc::scoped_ptr<rtc::Thread> decoder_thread_;  //thread on which the decoder will be working on.
    IMFTransform* decoder_;

    typedef std::list<IMFSample*> PendingInputs;
    PendingInputs pending_input_buffer;

    struct PendingSampleInfo {
        PendingSampleInfo(int32_t buffer_id, IMFSample* sample);
        ~PendingSampleInfo();

        int32_t input_buffer_id;
        IMFSample* output_sample;
    };

    typedef std::list<PendingSampleInfo> PendingOutputSamples;
    PendingOutputSamples pending_output_samples_;

    rtc::CriticalSection critical_section_;
    IDirect3D9Ex* d3d9_;
    IDirect3DDevice9Ex* device_;
    IDirect3DDeviceManager9* dev_manager_;
    IDirect3DQuery9* query_;
    NativeD3DSurfaceHandleImpl native_handle_;
    bool key_frame_required_;
    bool inited_;
    volatile State state_;

    int stride_;
    const GUID output_format_;
    int width_;
    int height_;

    int in_buffer_size_;  //the minimum buffer size for input
    int out_buffer_size_;
    webrtc::VideoCodec codec_;
    //webrtc::I420VideoFrame decoded_image_;
    webrtc::I420BufferPool decoded_frame_pool_;
    webrtc::DecodedImageCallback* callback_;
    webrtc::CriticalSectionWrapper& timestampCS_;
    std::vector<int64_t> ntp_time_ms_;
    std::vector<int32_t> timestamps_;
    HWND decoder_wnd_;
};