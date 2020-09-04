// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WIN_MSDKVIDEODECODER_H_
#define OWT_BASE_WIN_MSDKVIDEODECODER_H_

#include <codecapi.h>
#include <combaseapi.h>
#include <d3d11.h>
#include <dxva2api.h>
#include <memory>
#include <utility>
#include <vector>
#include "webrtc/media/base/codec.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/critical_section.h"
#include "webrtc/rtc_base/thread.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/base/win/msdkvideobase.h"
#include "talk/owt/sdk/base/win/d3d11_allocator.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"

namespace owt {
namespace base {

//
// MSDK Video Decoder declaration.
//
class MSDKVideoDecoder : public webrtc::VideoDecoder {
public:
    enum State {
        kUnitialized,
        kNormal,
        kResetting,
        kStopped,
        kFlushing,
    };
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
private:
    int32_t InitDecodeOnCodecThread();
    void CheckOnCodecThread();
    bool CreateD3D11Device();
    int32_t Reset();

    // Type of video codec.
    webrtc::VideoCodecType codec_type;
    mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);
    void WipeMfxBitstream(mfxBitstream* pBitstream);
    void ReadFromInputStream(mfxBitstream* pBitstream, const uint8_t *data, size_t len);
    mfxU16 DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxU16 DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);

    // Begin MSDK variables
    MFXVideoSession* m_mfxSession;
    std::unique_ptr<MFXVideoDECODE> m_pmfxDEC;
    std::shared_ptr<D3D11FrameAllocator> m_pMFXAllocator;
    mfxVideoParam           m_mfxVideoParams;
    mfxBitstream            m_mfxBS; // Contains encoded data
    mfxFrameAllocResponse   m_mfxResponse; // Memory allocation response for decoder
    mfxFrameSurface1*       m_pInputSurfaces;
    mfxPluginUID            m_pluginID;
    bool                    m_video_param_extracted;
    uint32_t                m_decBsOffset;
    // End of MSDK variables
    CComPtr<ID3D11Device> d3d11_device;
    CComPtr<ID3D11DeviceContext> d3d11_device_context;
    CComPtr<ID3D11VideoDevice> d3d11_video_device;
    CComPtr<ID3D11VideoContext> d3d11_video_context;
    // Store current decoded frame.
    std::unique_ptr<D3D11ImageHandle> surface_handle;

    bool inited;
    int width;
    int height;
    std::unique_ptr<rtc::Thread> decoder_thread;  // Thread on which the decoder will be working on.
    webrtc::VideoCodec codec_;

    webrtc::DecodedImageCallback* callback_;
    rtc::CriticalSection timestampCS_;
    std::vector<int64_t> ntp_time_ms_;
    std::vector<int32_t> timestamps_;
};
}  // namespace base
}  // namespace owt
#endif // OWT_BASE_WIN_MSDKVIDEODECODER_H_
