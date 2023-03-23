// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_D3D11VA_H264_DECODER_H_
#define OWT_BASE_WIN_D3D11VA_H264_DECODER_H_

#include <map>
#include <memory>
#include "modules/video_coding/codecs/h264/include/h264.h"
#include <unordered_map>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/error.h>
}  // extern "C"

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

// For dxva only.
#include <libavcodec/d3d11va.h>
#include <libavutil/hwcontext_d3d11va.h>

#include "talk/owt/sdk/base/win/d3d11device.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"

namespace owt {
namespace base {

class H264DXVADecoderImpl : public webrtc::H264Decoder {
 public:
  static std::unique_ptr<H264DXVADecoderImpl> Create(
      cricket::VideoCodec format);
  H264DXVADecoderImpl(ID3D11Device* external_device);
  ~H264DXVADecoderImpl() override;

  int32_t InitDecode(const webrtc::VideoCodec* codec_settings,
                     int32_t number_of_cores) override;
  int32_t Release() override;

  int32_t RegisterDecodeCompleteCallback(
      webrtc::DecodedImageCallback* callback) override;

  int32_t Decode(const webrtc::EncodedImage& input_image,
                 bool /*missing_frames*/,
                 int64_t render_time_ms = -1) override;

  const char* ImplementationName() const override;

 private:
  bool IsInitialized() const;
  int InitHwContext(AVCodecContext* ctx, const enum AVHWDeviceType type);
  int PrepareHwDecoder(webrtc::VideoCodecType codec_type);
  // Reports statistics with histograms.
  void ReportInit();
  void ReportError();
  int64_t GetSideData(const uint8_t* frame_data,
                      size_t frame_length,
                      std::vector<uint8_t>& side_data,
                      std::vector<uint8_t>& cursor_data);
  webrtc::DecodedImageCallback* decoded_image_callback_;

  bool has_reported_init_;
  bool has_reported_error_;

  // 
  CComPtr<ID3D11Device> d3d11_device_;
  CComPtr<ID3D11DeviceContext> d3d11_device_context_;
  CComPtr<ID3D11VideoDevice> d3d11_video_device_;
  CComPtr<ID3D11VideoContext> d3d11_video_context_;
  CComQIPtr<IDXGIAdapter> m_padapter_;
  CComPtr<IDXGIFactory2> m_pdxgi_factory_;
  std::unique_ptr<D3D11VAHandle> surface_handle_;
  std::vector<uint8_t> current_side_data_;
  std::unordered_map<uint32_t, std::vector<uint8_t>> side_data_list_;
  // cursor data is allowed to be overriden, so not keeping record per-frame.
  std::vector<uint8_t> current_cursor_data_;
  AVBufferRef* hw_device_ctx = nullptr;
  AVCodecContext* decoder_ctx = nullptr;
  const AVCodec* decoder = nullptr;
  webrtc::Clock* clock_ = nullptr;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_WIN_D3D11VA_H264_DECODER_H_
