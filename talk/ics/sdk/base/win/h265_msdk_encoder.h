/*
* Intel License
*/
#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_H265_VIDEO_MFT_ENCODER_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_H265_VIDEO_MFT_ENCODER_H_

#include "base_allocator.h"
#include "mfxvideo.h"
#include "mfxvideo++.h"
#include "mfxplugin++.h"
#include <vector>
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include "webrtc/rtc_base/thread.h"

// This file provides a H264 encoder implementation using the WMF
// APIs.
enum H265MemType {
    H265_SYSTEM_MEMORY = 0x00,
    H265_D3D9_MEMORY = 0x01,
    H265_D3D11_MEMORY = 0x02,
};

class H265EncoderThread : public rtc::Thread {
public:
    virtual void Run();
    ~H265EncoderThread() override;
};

class H265VideoMFTEncoder : public webrtc::VideoEncoder {
public:
    H265VideoMFTEncoder();

    ~H265VideoMFTEncoder() override;

    int InitEncode(const webrtc::VideoCodec* codec_settings,
        int number_of_cores,
        size_t max_payload_size) override;

    int Encode(const webrtc::VideoFrame& input_image,
        const webrtc::CodecSpecificInfo* codec_specific_info,
        const std::vector<webrtc::FrameType>* frame_types) override;

    int RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override;

    int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;

    int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;

    bool SupportsNativeHandle() const { return false; }

    int Release() override;

private:
    int InitEncodeOnEncoderThread(const webrtc::VideoCodec* codec_settings,
        int number_of_cores,
        size_t max_payload_size);
    void CheckOnEncoderThread();
    int EncodeOnEncoderThread(const webrtc::VideoFrame& frame, const webrtc::CodecSpecificInfo* codec_specific_info,
        const std::vector<webrtc::FrameType>* frame_types);
    // Search for H.264 start codes.
    int32_t NextNaluPosition(uint8_t *buffer, size_t buffer_size, uint8_t *sc_length);
    mfxU16 H265VideoMFTEncoder::H265GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxU16 H265VideoMFTEncoder::H265GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    void WipeMfxBitstream(mfxBitstream* pBitstream);
    webrtc::EncodedImageCallback* callback_;
    int32_t bitrate_;  // Bitrate in bits per second.
    int32_t width_;
    int32_t height_;
    int32_t framerate_;
    webrtc::VideoCodecType codecType_;

    MFXVideoSession m_mfxSession_;
    MFXVideoENCODE* m_pmfxENC;
    MFXFrameAllocator* m_pMFXAllocator;
    mfxAllocatorParams* m_pmfxAllocatorParams;
    mfxVideoParam m_mfxEncParams;
    mfxExtHEVCParam m_ExtHEVCParam;
    std::vector<mfxExtBuffer*> m_EncExtParams;
    mfxFrameAllocResponse m_EncResponse;
    mfxFrameSurface1* m_pEncSurfaces; // frames array for encoder
    std::auto_ptr<MFXPlugin> m_hevc_plugin_;
    mfxBitstream            m_mfxBS; // contains encoded data
    mfxU32 m_nFramesProcessed;
    std::unique_ptr<H265EncoderThread> encoder_thread_;
    H265MemType m_memType_;
    bool inited_;
#ifdef ICS_DEBUG_H265_ENC
    FILE *output;
    FILE *input;
    FILE *raw_in;
#endif
};  // H265VideoMFTEncoder
#endif  // WEBRTC_MODULES_VIDEO_CODING_CODECS_H265_H265_VIDEO_MFT_ENCODER_H_
