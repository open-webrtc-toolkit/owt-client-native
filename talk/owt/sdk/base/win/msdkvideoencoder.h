// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEOENCODER_H_
#define OWT_BASE_WIN_MSDKVIDEOENCODER_H_

#include <cstddef>
#include <vector>
#include "base_allocator.h"
#include "mfxplugin++.h"
#include "mfxvideo++.h"
#include "mfxvideo.h"
#include "sysmem_allocator.h"
#include "webrtc/api/video_codecs/video_codec.h"
#include "webrtc/media/base/codec.h"
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include "webrtc/rtc_base/thread.h"
#include "talk/owt/sdk/base/win/mediacapabilities.h"

namespace owt {
namespace base {
enum MemType {
  MSDK_SYSTEM_MEMORY = 0x00,
  MSDK_D3D9_MEMORY = 0x01,
  MSDK_D3D11_MEMORY = 0x02,
};

/// Encoder with Intel MediaSDK as the backend.
class MSDKVideoEncoder : public webrtc::VideoEncoder {
 public:
  explicit MSDKVideoEncoder(const cricket::VideoCodec& codec);
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
  webrtc::VideoCodecType codec_type;
  uint32_t num_temporal_layers;
  uint32_t num_spatial_layers = 1;  // For MSDK this is fixed to 1;
  webrtc::InterLayerPredMode inter_layer_prediction_mode;

  std::unique_ptr<MFXVideoSession> m_mfxSession;
  // TODO: we should probably remove this.
  mfxPluginUID m_pluginID;
  std::unique_ptr<MFXVideoENCODE> m_pmfxENC;
  std::shared_ptr<SysMemFrameAllocator> m_pMFXAllocator;
  mfxVideoParam m_mfxEncParams;

  // Used by HEVC
  mfxExtHEVCParam m_ExtHEVCParam;
  // H265Profile space is always 0.
  H265ProfileId h265_profile; 

  // Used by VP9
  mfxExtVP9Param vp9_ext_param;
  VP9Profile vp9_profile;


  // TODO: change to actual version when turning this on.
#if (MFX_VERSION >= MFX_VERSION_NEXT)
  mfxExtAV1Param av1_ext_param;
  AV1Profile av1_profile;
#endif

  std::vector<mfxExtBuffer*> m_EncExtParams;
  mfxFrameAllocResponse m_EncResponse;
  mfxFrameSurface1* m_pEncSurfaces;  // frames array for encoder
  mfxU32 m_nFramesProcessed;
  std::unique_ptr<rtc::Thread> encoder_thread;
  std::atomic<bool> inited;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEOENCODER_H_
