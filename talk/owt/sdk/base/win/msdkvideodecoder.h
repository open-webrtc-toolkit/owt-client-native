// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WIN_MSDKVIDEODECODER_H_
#define OWT_BASE_WIN_MSDKVIDEODECODER_H_

#include <utility>
#include <vector>

#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/critical_section.h"
#include "webrtc/rtc_base/thread.h"

#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/common_video/include/i420_buffer_pool.h"

// Solve conflict in FOURCC def with libyuv
#ifdef FOURCC
#undef FOURCC
#endif
#include "third_party/libyuv/include/libyuv.h"
#include "webrtc/media/base/codec.h"
#include "talk/owt/sdk/base/win/d3d11device.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <dxva2api.h>
#include "msdkvideobase.h"
#include "base_allocator.h"
#include "d3d11_allocator.h"

namespace owt {
namespace base {

//
// MSDK Video Decoder declaration.
//
class MSDKVideoDecoder : public webrtc::VideoDecoder,
    public rtc::MessageHandler {
public:
    explicit MSDKVideoDecoder();
    virtual ~MSDKVideoDecoder();

    static std::unique_ptr<MSDKVideoDecoder> Create(cricket::VideoCodec format);

    int32_t InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores)
        override;

    int32_t Decode(
        const webrtc::EncodedImage& inputImage, bool missingFrames,
        int64_t renderTimeMs = -1) override;

    int32_t RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
        override;

    int32_t Release() override;

    const char* ImplementationName() const override;
    // rtc::MessageHandler implementation.
    void OnMessage(rtc::Message* msg) override;
private:
    int32_t InitDecodeOnCodecThread();
    void CheckOnCodecThread();
    int32_t Reset();

    mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);
    void WipeMfxBitstream(mfxBitstream* pBitstream);
    void ReadFromInputStream(mfxBitstream* pBitstream, const uint8_t *data, size_t len);
    mfxU16 DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxU16 DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxStatus CreateD3D11Resources();

    // Begin MSDK variables
    MFXVideoSession*        mfx_session_;
    std::unique_ptr<MFXVideoDECODE> msdk_dec_;
    mfxVideoParam           mfx_video_param_;
    mfxBitstream            mfx_bs_; // Contains encoded data
    mfxFrameAllocResponse   mfx_alloc_response_; // Memory allocation response for decoder
    mfxFrameSurface1*       mfx_input_surfaces_;
    mfxPluginUID            mfx_plugin_id_;
    bool                    mfx_video_param_extracted_;
    uint32_t                mfx_dec_bs_offset_;
    // End of MSDK variables

    // Begin D3D11 variables
    std::unique_ptr<RTCD3D11Device> d3d11_device_;
    std::unique_ptr<D3D11FrameAllocator> d3d11_frame_allocator_;
    std::unique_ptr<D3D11AllocatorParams> d3d11_allocator_param_;
    std::unique_ptr<D3D11Handle> surface_handle_;
    // End D3D11 variables

    bool inited_;
    std::unique_ptr<rtc::Thread> decoder_thread_;
    webrtc::VideoCodec codec_;

    webrtc::DecodedImageCallback* callback_;
    std::vector<int64_t> ntp_time_ms_;
    std::vector<int32_t> timestamps_;
#ifdef OWT_DEBUG_DEC
    FILE *input;
#endif
};
}  // namespace base
}  // namespace owt
#endif // OWT_BASE_WIN_MSDKVIDEODECODER_H_
