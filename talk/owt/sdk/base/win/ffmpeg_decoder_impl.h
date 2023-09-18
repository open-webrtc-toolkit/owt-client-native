// Copyright (C) <2023> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_FFMPEG_DECODER_IMPL_H_
#define OWT_BASE_WIN_FFMPEG_DECODER_IMPL_H_

#include <memory>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "modules/video_coding/codecs/h265/include/h265_globals.h"

extern "C" {
#include "third_party/ffmpeg/libavcodec/avcodec.h"
}  // extern "C"

#include "api/video_codecs/video_decoder.h"
#include "common_video/h265/h265_bitstream_parser.h"
#include "common_video/include/video_frame_buffer_pool.h"

#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"

namespace webrtc {
class Clock;
}

namespace owt {
namespace base {

struct AVCodecContextDeleter {
  void operator()(AVCodecContext* ptr) const { avcodec_free_context(&ptr); }
};
struct AVFrameDeleter {
  void operator()(AVFrame* ptr) const { av_frame_free(&ptr); }
};

class FFMpegDecoderImpl : public webrtc::VideoDecoder {
 public:
  FFMpegDecoderImpl();
  ~FFMpegDecoderImpl() override;

  bool Configure(const webrtc::VideoDecoder::Settings& settings) override;
  int32_t Release() override;

  int32_t RegisterDecodeCompleteCallback(
      webrtc::DecodedImageCallback* callback) override;

  // `missing_frames`, `fragmentation` and `render_time_ms` are ignored.
  int32_t Decode(const webrtc::EncodedImage& input_image,
                 bool /*missing_frames*/,
                 int64_t render_time_ms = -1) override;

  const char* ImplementationName() const override;

 private:
  // Called by FFmpeg when it needs a frame buffer to store decoded frames in.
  // The `VideoFrame` returned by FFmpeg at `Decode` originate from here. Their
  // buffers are reference counted and freed by FFmpeg using `AVFreeBuffer2`.
  static int AVGetBuffer2(AVCodecContext* context,
                          AVFrame* av_frame,
                          int flags);
  // Called by FFmpeg when it is done with a video frame, see `AVGetBuffer2`.
  static void AVFreeBuffer2(void* opaque, uint8_t* data);

  void CreateStagingTextureIfNeeded(int width, int height);

  bool IsInitialized() const;

  // Reports statistics with histograms.
  void ReportInit();
  void ReportError();

  // Used by ffmpeg via `AVGetBuffer2()` to allocate I420/I444 images.
  webrtc::VideoFrameBufferPool ffmpeg_buffer_pool_;
  std::unique_ptr<AVCodecContext, AVCodecContextDeleter> av_context_;
  std::unique_ptr<AVFrame, AVFrameDeleter> av_frame_;
  CComPtr<ID3D11Device> d3d11_device_;
  CComPtr<ID3D11DeviceContext> d3d11_device_context_;
  CComPtr<ID3D11VideoDevice> d3d11_video_device_;
  CComPtr<ID3D11VideoContext> d3d11_video_context_;
  CComQIPtr<IDXGIAdapter> m_padapter_;
  CComPtr<IDXGIFactory2> m_pdxgi_factory_;
  CComPtr<ID3D11Texture2D> staging_texture_;
  CComPtr<ID3D11Texture2D> output_texture_;
  std::unique_ptr<D3D11VAHandle> surface_handle_;

  webrtc::DecodedImageCallback* decoded_image_callback_;

  bool has_reported_init_;
  bool has_reported_error_;

  webrtc::H265BitstreamParser h265_bitstream_parser_;
  webrtc::Clock* clock_ = nullptr;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_WIN_FFMPEG_DECODER_IMPL_H_