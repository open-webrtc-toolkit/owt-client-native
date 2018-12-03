/*
Copyright (C) 2018 Intel Corporation. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef OMS_BASE_WIN_MSDKVIDEOENCODER_H_
#define OMS_BASE_WIN_MSDKVIDEOENCODER_H_

#include "base_allocator.h"
#include "mfxvideo.h"
#include "mfxvideo++.h"
#include "mfxplugin++.h"
#include "sysmem_allocator.h"
#include <vector>
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include "webrtc/rtc_base/thread.h"

namespace oms {
namespace base {
enum MemType {
  MSDK_SYSTEM_MEMORY = 0x00,
  MSDK_D3D9_MEMORY = 0x01,
  MSDK_D3D11_MEMORY = 0x02,
};

class MSDKEncoderThread : public rtc::Thread {
 public:
  virtual void Run() override;
  ~MSDKEncoderThread() override;
};

class MSDKVideoEncoder : public webrtc::VideoEncoder {
 public:
  MSDKVideoEncoder();
  ~MSDKVideoEncoder() override;
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
  int32_t NextNaluPosition(uint8_t *buffer, size_t buffer_size, uint8_t *sc_length);
  mfxU16 MSDKGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
  mfxU16 MSDKGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);
  void WipeMfxBitstream(mfxBitstream* pBitstream);
  
  webrtc::EncodedImageCallback* callback_;
  int32_t bitrate_;  // Bitrate in bits per second.
  int32_t width_;
  int32_t height_;
  int32_t framerate_;
  webrtc::VideoCodecType codecType_;

  MFXVideoSession* m_mfxSession;
  mfxPluginUID m_pluginID;
  MFXVideoENCODE* m_pmfxENC;
  std::shared_ptr<SysMemFrameAllocator> m_pMFXAllocator;
  mfxVideoParam m_mfxEncParams;
  mfxExtHEVCParam m_ExtHEVCParam;
  std::vector<mfxExtBuffer*> m_EncExtParams;
  mfxFrameAllocResponse m_EncResponse;
  mfxFrameSurface1* m_pEncSurfaces; // frames array for encoder
  mfxBitstream            m_mfxBS; // contains encoded data
  mfxU32 m_nFramesProcessed;
  std::unique_ptr<MSDKEncoderThread> encoder_thread_;
  MemType m_memType_;
  bool inited_;
#ifdef OMS_DEBUG_MSDK_ENC
  FILE *output;
  FILE *input;
  FILE *raw_in;
#endif
};
}  // namespace base
}  // namespace oms
#endif  // OMS_BASE_WIN_MSDKVIDEOENCODER_H_
