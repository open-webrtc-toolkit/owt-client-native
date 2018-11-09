/*
* Intel License
*/

#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_VIDEO_MFT_ENCODER_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_VIDEO_MFT_ENCODER_H_

#include "base_allocator.h"
#include "mfxvideo.h"
#include "mfxvideo++.h"
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include <vector>
#include "webrtc/rtc_base/thread.h"
// This file provides a H264 encoder implementation using the WMF
// APIs.
enum MemType {
    SYSTEM_MEMORY = 0x00,
    D3D9_MEMORY = 0x01,
    D3D11_MEMORY = 0x02,
};
class MFTEncoderThread : public rtc::Thread {
public:
    virtual void Run() override;
    ~MFTEncoderThread() override;
};
class H264VideoMFTEncoder : public webrtc::H264Encoder {
public:
    H264VideoMFTEncoder();
    ~H264VideoMFTEncoder() override;
    int InitEncode(const webrtc::VideoCodec* codec_settings,
        int number_of_cores,
        size_t max_payload_size) override;
    int Encode(const webrtc::VideoFrame& input_image,
        const webrtc::CodecSpecificInfo* codec_specific_info,
        const std::vector<webrtc::FrameType>* frame_types) override;
    int RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback) override;
    int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;
    int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;
    bool SupportsNativeHandle() const override { return false; }
    int Release() override;
private:
    int InitEncodeOnEncoderThread(const webrtc::VideoCodec* codec_settings,
        int number_of_cores,
        size_t max_payload_size);
    void CheckOnEncoderThread();
    int EncodeOnEncoderThread(const webrtc::VideoFrame& frame, const webrtc::CodecSpecificInfo* codec_specific_info,
        const std::vector<webrtc::FrameType>* frame_types);
    // Search for H.264 start codes.
    int32_t NextNaluPosition(uint8_t *buffer, size_t buffer_size, size_t *sc_length);
    mfxU16 H264VideoMFTEncoder::H264GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    mfxU16 H264VideoMFTEncoder::H264GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
    webrtc::EncodedImageCallback* callback_;
    int32_t bitrate_;  // Bitrate in bits per second.
    int32_t max_bitrate_;
    int32_t width_;
    int32_t height_;
    int32_t framerate_;
    webrtc::VideoCodecType codecType_;
    MFXVideoSession m_mfxSession_;
    MFXVideoENCODE* m_pmfxENC;
    MFXFrameAllocator* m_pMFXAllocator;
    mfxAllocatorParams* m_pmfxAllocatorParams;
    mfxVideoParam m_mfxEncParams;
    mfxFrameAllocResponse m_EncResponse;
    mfxFrameSurface1* m_pEncSurfaces; // frames array for encoder
    mfxU32 m_nFramesProcessed;
    MemType m_memType_;
    std::unique_ptr<MFTEncoderThread> encoder_thread_;
    bool inited_;
#ifdef OMS_DEBUG_H264_ENC
    FILE *output;
#endif
};  // H264VideoMFTEncoder
#endif  // WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_VIDEO_MFT_ENCODER_H_
