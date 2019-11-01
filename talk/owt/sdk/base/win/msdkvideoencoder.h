// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEOENCODER_H_
#define OWT_BASE_WIN_MSDKVIDEOENCODER_H_

#include <vector>
#include "base_allocator.h"
#include "mfxplugin++.h"
#include "mfxvideo++.h"
#include "mfxvideo.h"
#include "sysmem_allocator.h"
#include "webrtc/media/base/codec.h"
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include "webrtc/rtc_base/thread.h"

namespace owt {
namespace base {
enum MemType {
  MSDK_SYSTEM_MEMORY = 0x00,
  MSDK_D3D9_MEMORY = 0x01,
  MSDK_D3D11_MEMORY = 0x02,
};

class MSDKVideoEncoder : public webrtc::VideoEncoder {
 public:
  explicit MSDKVideoEncoder();
  virtual ~MSDKVideoEncoder();

  static std::unique_ptr<MSDKVideoEncoder> Create(cricket::VideoCodec format);
  int InitEncode(const webrtc::VideoCodec* codec_settings,
                 int number_of_cores,
                 size_t max_payload_size) override;
  int Encode(const webrtc::VideoFrame& input_image,
             const std::vector<webrtc::VideoFrameType>* frame_types) override;
  int RegisterEncodeCompleteCallback(
      webrtc::EncodedImageCallback* callback) override;
  void SetRates(const RateControlParameters& parameters) override;
  void OnPacketLossRateUpdate(float packet_loss_rate) override;
  void OnRttUpdate(int64_t rtt_ms) override;
  void OnLossNotification(const LossNotification& loss_notification) override;
  EncoderInfo GetEncoderInfo() const override;
  int Release() override;

 private:
  int InitEncodeOnEncoderThread(const webrtc::VideoCodec* codec_settings,
                                int number_of_cores,
                                size_t max_payload_size);
  int32_t NextNaluPosition(uint8_t* buffer,
                           size_t buffer_size,
                           uint8_t* sc_length);
  mfxU16 MSDKGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
  mfxU16 MSDKGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool,
                                 mfxU16 nPoolSize);
  void WipeMfxBitstream(mfxBitstream* pBitstream);

  webrtc::EncodedImageCallback* callback_;
  int32_t bitrate_;  // Bitrate in bits per second.
  int32_t width_;
  int32_t height_;
  webrtc::VideoCodecType codecType_;

  MFXVideoSession* m_mfxSession;
  mfxPluginUID m_pluginID;
  MFXVideoENCODE* m_pmfxENC;
  std::shared_ptr<SysMemFrameAllocator> m_pMFXAllocator;
  mfxVideoParam m_mfxEncParams;
  mfxExtHEVCParam m_ExtHEVCParam;
  std::vector<mfxExtBuffer*> m_EncExtParams;
  mfxFrameAllocResponse m_EncResponse;
  mfxFrameSurface1* m_pEncSurfaces;  // frames array for encoder
  mfxU32 m_nFramesProcessed;
  std::unique_ptr<rtc::Thread> encoder_thread_;
  bool inited_;
#ifdef OWT_DEBUG_MSDK_ENC
  FILE* output;
  FILE* input;
  FILE* raw_in;
#endif
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEOENCODER_H_
