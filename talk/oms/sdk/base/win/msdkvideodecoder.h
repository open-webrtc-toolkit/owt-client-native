/*
* Intel License
*/
#ifndef OMS_BASE_WIN_MSDKVIDEODECODER_H_
#define OMS_BASE_WIN_MSDKVIDEODECODER_H_

#include <utility>
#include <vector>

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/thread.h"

// For decoder and encoder factory
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/common_video/include/i420_buffer_pool.h"

// Solve conflict in FOURCC def with libyuv
#ifdef FOURCC
#undef FOURCC
#endif
#include "third_party/libyuv/include/libyuv.h"

#include "talk/oms/sdk/base/win/d3dnativeframe.h"
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <dxva2api.h>

#include "msdkvideobase.h"
#include "base_allocator.h"

namespace oms {
namespace base {

//#define OMS_DEBUG_DEC

//
// MSDK Video Decoder declaration.
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
    MSDKVideoDecoder(webrtc::VideoCodecType codecType);
    virtual ~MSDKVideoDecoder();

    int32_t InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores)
        override;

    int32_t Decode(
        const webrtc::EncodedImage& inputImage, bool missingFrames,
        const webrtc::CodecSpecificInfo* codecSpecificInfo = NULL,
        int64_t renderTimeMs = -1) override;

    int32_t RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
        override;

    int32_t Release() override;

    // rtc::MessageHandler implementation.
    void OnMessage(rtc::Message* msg) override;
private:
    int32_t InitDecodeOnCodecThread();
    void CheckOnCodecThread();
    bool CreateD3DDevice();

    // Type of video codec.
    webrtc::VideoCodecType codecType_;
    mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);
    void WipeMfxBitstream(mfxBitstream* pBitstream);
    void ReadFromInputStream(mfxBitstream* pBitstream, uint8_t *data, size_t len);
    mfxU16 DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxU16 DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);

    // Begin MSDK variables
    MFXVideoSession*        m_mfxSession;
    MFXVideoDECODE*         m_pmfxDEC;
    std::shared_ptr<D3DFrameAllocator> m_pMFXAllocator;
    mfxVideoParam           m_mfxVideoParams;
    mfxBitstream            m_mfxBS; // Contains encoded data
    mfxFrameAllocResponse   m_mfxResponse; // Memory allocation response for decoder
    mfxFrameSurface1*       m_pInputSurfaces;
    mfxPluginUID            m_pluginID;
    bool                    m_video_param_extracted;
    // End of MSDK variables
    IDirect3D9Ex*               m_pD3D9;
    IDirect3DDevice9Ex*         m_pD3DD9;
    IDirect3DDeviceManager9*    d3d_manager;
    D3DPRESENT_PARAMETERS       present_params;
    UINT                        m_resetToken;;

    rtc::CriticalSection critical_section_;
    bool inited_;
    int width_;
    int height_;
    std::unique_ptr<rtc::Thread> decoder_thread_;  // Thread on which the decoder will be working on.
    webrtc::VideoCodec codec_;

    webrtc::I420BufferPool decoded_frame_pool_;
    webrtc::DecodedImageCallback* callback_;
    rtc::CriticalSection timestampCS_;
    std::vector<int64_t> ntp_time_ms_;
    std::vector<int32_t> timestamps_;
#ifdef OMS_DEBUG_DEC
    FILE *input;
#endif
};
}  // namespace base
}  // namespace oms
#endif // OMS_BASE_WIN_MSDKVIDEODECODER_H_
