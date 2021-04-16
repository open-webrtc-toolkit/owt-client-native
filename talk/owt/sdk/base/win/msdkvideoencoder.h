// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEOENCODER_H_
#define OWT_BASE_WIN_MSDKVIDEOENCODER_H_

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>
#include "base_allocator.h"
#include "mfxplugin++.h"
#include "mfxvideo++.h"
#include "mfxvideo.h"
#include "sysmem_allocator.h"
#include "modules/video_coding/utility/ivf_file_writer.h"
#include "webrtc/api/video_codecs/video_codec.h"
#include "webrtc/api/video_codecs/video_encoder.h"
#include "webrtc/media/base/codec.h"
#include "webrtc/media/base/vp9_profile.h"
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include "webrtc/rtc_base/thread.h"
#include "talk/owt/sdk/base/win/mediacapabilities.h"
#include "talk/owt/sdk/base/win/vp9ratecontrol.h"
#include "vp9/ratectrl_rtc.h"

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
  libvpx::VP9RateControlRtcConfig CreateVP9RateControlConfig();

  webrtc::EncodedImageCallback* callback_;
  int32_t bitrate_;  // Bitrate in bits per second.
  int32_t width_;
  int32_t height_;
  uint32_t frame_rate;
  webrtc::VideoCodecType codec_type;
  cricket::VideoCodec rtp_codec_parameters;
  uint8_t num_temporal_layers = 1;
  uint32_t num_spatial_layers = 1;  // For MSDK this is fixed to 1;
  webrtc::InterLayerPredMode inter_layer_prediction_mode;

  MFXVideoSession* m_mfxSession;
  std::unique_ptr<MFXVideoENCODE> m_pmfxENC;
  std::shared_ptr<SysMemFrameAllocator> m_pMFXAllocator;
  mfxVideoParam m_mfxEncParams;

  // Members used by HEVC
  mfxExtHEVCParam m_ExtHEVCParam;
  // H265Profile space is always 0.
  H265ProfileId h265_profile = owt::base::H265ProfileId::kMain; 

  // Members used by VP9
  mfxExtVP9Param vp9_ext_param;
  webrtc::VP9Profile vp9_profile = webrtc::VP9Profile::kProfile0;
  std::unique_ptr<VP9RateControl> vp9_rate_ctrl;
  libvpx::VP9RateControlRtcConfig vp9_rc_config;
  libvpx::VP9FrameParamsQpRTC frame_params;
  bool vp9_use_external_brc = false;

  // TODO(johny): MSDK will remove the version macro usage for headers.
  // Turn this on when appropriate.
#if (MFX_VERSION >= MFX_VERSION_NEXT)
  mfxExtAV1Param av1_ext_param;
  AV1Profile av1_profile = owt::base::AV1Profile::kMain;
#endif

  std::vector<mfxExtBuffer*> m_EncExtParams;
  mfxFrameAllocResponse m_EncResponse;
  mfxFrameSurface1* m_pEncSurfaces;  // frames array for encoder
  mfxU32 m_nFramesProcessed;
  std::unique_ptr<rtc::Thread> encoder_thread;
  std::atomic<bool> inited;

  // Gof related information for VP9 codec specific info.
  uint8_t gof_idx;
  webrtc::GofInfoVP9 gof;
  std::unique_ptr<webrtc::IvfFileWriter> dump_writer;
  bool enable_bitstream_dump = false;
  std::string encoder_dump_file_name;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEOENCODER_H_
