// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_LINUX_MSDKVIDEODECODER_H_
#define OWT_BASE_LINUX_MSDKVIDEODECODER_H_

#include "webrtc/api/video_codecs/video_encoder.h"
#include "webrtc/api/video_codecs/sdp_video_format.h"
#include "webrtc/common_video/include/i420_buffer_pool.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/thread.h"

#include "displayutils.h"
#include "msdkvideosession.h"

namespace owt {
namespace base {

typedef struct msdkFrameSurface2 {
  mfxFrameSurface1 * frame;
  mfxU16 render_lock;
} msdkFrameSurface2;

class MsdkVideoDecoder : public webrtc::VideoDecoder {
 public:
  MsdkVideoDecoder();
  ~MsdkVideoDecoder() override;

  int32_t InitDecode(const webrtc::VideoCodec* codec_settings,
                     int32_t number_of_cores) override;

  int32_t Decode(const webrtc::EncodedImage& inputImage,
                 bool missingFrames,
                 int64_t renderTimeMs = -1) override;


  int32_t RegisterDecodeCompleteCallback(
      webrtc::DecodedImageCallback* callback) override;

  int32_t Release() override;

  const char* ImplementationName() const override {
    return "Intel MediaSDK";
  }

  msdkFrameSurface2* pmsdkDecSurfaces;
 private:
  int32_t InitDecodeOnDecoderThread();
  int32_t InitVideoDecoder();

  mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);
  void WipeMfxBitstream(mfxBitstream* pBitstream);
  void ReadFromInputStream(mfxBitstream* pBitstream, const uint8_t* data, size_t len);
  mfxU16 DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool,
                               mfxU16 nPoolSize);
  mfxU16 DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool,
                                    mfxU16 nPoolSize);
  mfxU16 DecGetFreeSurface2(msdkFrameSurface2* pSurfacesPool,
                               mfxU16 nPoolSize);
  mfxU16 DecGetFreeSurfaceIndex2(msdkFrameSurface2* pSurfacesPool,
                                    mfxU16 nPoolSize);

  bool inited_;
  webrtc::VideoCodec codec_settings_;

  std::shared_ptr<mfxFrameAllocator> frame_allocator_;
  std::unique_ptr<mfxVideoParam> video_parameter_;
  std::unique_ptr<mfxBitstream> bit_stream_;
  mfxFrameAllocResponse allocate_response_;
  mfxFrameSurface1* input_surfaces_;
  mfxFrameAllocRequest allocate_request_;

  bool video_param_extracted_;
  uint32_t bit_stream_offset_;
  uint32_t search_start;

  std::unique_ptr<rtc::Thread> decoder_thread_;
  MFXVideoSession* decoder_session_;
  MFXVideoDECODE* decoder_;

  webrtc::DecodedImageCallback* callback_;
  unsigned int frame_num = 0;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_LINUX_MSDKVIDEODECODER_H_
