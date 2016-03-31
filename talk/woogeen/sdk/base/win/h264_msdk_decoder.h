/*
* Intel License
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
#include <d3d9.h>
#include <dxva2api.h>
#include "base_allocator.h"
#include "mfxvideo.h"
#include "mfxvideo++.h"
#include "mfx_buffering.h"

#pragma once
//
//MSDK Video Decoder declaration.
//

class H264MSDKVideoDecoder : public webrtc::VideoDecoder,
    public rtc::MessageHandler {
public:
    enum State{
        kUnitialized,
        kNormal,
        kResetting,
        kStopped,
        kFlushing,
    };
    H264MSDKVideoDecoder(webrtc::VideoCodecType codecType, HWND decoder_window);
    virtual ~H264MSDKVideoDecoder();

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
private:
    int32_t InitDecodeOnCodecThread();
    void CheckOnCodecThread();
    bool CreateD3DDeviceManager();

    // Type of video codec.
    webrtc::VideoCodecType codecType_;
    rtc::scoped_ptr<rtc::Thread> decoder_thread_;  //thread on which the decoder will be working on.
    mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);
    void WipeMfxBitstream(mfxBitstream* pBitstream);
    void ReadFromInputStream(mfxBitstream* pBitstream, uint8_t *data, size_t len);
    mfxU16 H264DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxU16 H264DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    //Begin MSDK variables
    MFXVideoSession         m_mfxSession;
    MFXVideoDECODE*         m_pmfxDEC;
    mfxVideoParam           m_mfxVideoParams;
    mfxBitstream            m_mfxBS; // contains encoded data
    MFXFrameAllocator*      m_pMFXAllocator;
    mfxAllocatorParams*     m_pmfxAllocatorParams;
    mfxFrameAllocResponse   m_mfxResponse; // memory allocation response for decoder
    //msdkFrameSurface*        m_pInputSurfaces;
    mfxFrameSurface1*       m_pInputSurfaces;
    bool                    videoParamExtracted;

    //End of MSDK variables
    rtc::CriticalSection critical_section_;
    IDirect3D9Ex* d3d9_;
    IDirect3DDevice9Ex* device_;
    IDirect3DDeviceManager9* dev_manager_;
    IDirect3DQuery9* query_;
    NativeD3DSurfaceHandleImpl native_handle_;
    bool key_frame_required_;
    bool inited_;

    int stride_;
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