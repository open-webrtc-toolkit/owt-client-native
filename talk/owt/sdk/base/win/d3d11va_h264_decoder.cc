// Copyright (C) <20> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3d11va_h264_decoder.h"
#include <algorithm>
#include <limits>

#include "webrtc/api/video/color_space.h"
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/common_video/include/video_frame_buffer.h"
#include "webrtc/modules/video_coding/codecs/h264/h264_color_space.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/critical_section.h"
#include "webrtc/rtc_base/keep_ref_until_done.h"
#include "webrtc/rtc_base/logging.h"
#include "system_wrappers/include/metrics.h"

namespace owt {
namespace base {

namespace {

const AVPixelFormat kPixelFormatDefault = AV_PIX_FMT_YUV420P;
const AVPixelFormat kPixelFormatFullRange = AV_PIX_FMT_YUVJ420P;
const size_t kYPlaneIndex = 0;
const size_t kUPlaneIndex = 1;
const size_t kVPlaneIndex = 2;
static const int kMaxSideDataListSize = 20;

// Used by histograms. Values of entries should not be changed.
enum H264DecoderImplEvent {
  kH264DecoderEventInit = 0,
  kH264DecoderEventError = 1,
  kH264DecoderEventMax = 16,
};

}  // namespace

static const uint8_t frame_number_sei_guid[16] = {0xef, 0xc8, 0xe7, 0xb0, 0x26,
    0x26, 0x47, 0xfd, 0x9d, 0xa3, 0x49, 0x4f, 0x60, 0xb8, 0x5b, 0xf0};
static enum AVPixelFormat hw_pix_fmt;
static enum AVPixelFormat get_hw_format(AVCodecContext* ctx,
                                        const enum AVPixelFormat* pix_fmts) {
  const enum AVPixelFormat* p;

  for (p = pix_fmts; *p != -1; p++) {
    if (*p == hw_pix_fmt)
      return *p;
  }

  return AV_PIX_FMT_NONE;
}

H264DXVADecoderImpl::H264DXVADecoderImpl(ID3D11Device* external_device)
    : decoded_image_callback_(nullptr),
                                     has_reported_init_(false),
                                     has_reported_error_(false) {
  surface_handle_.reset(new D3D11VAHandle());
}

H264DXVADecoderImpl::~H264DXVADecoderImpl() {
  Release();
}

int H264DXVADecoderImpl::InitHwContext(AVCodecContext* ctx,
                                       const enum AVHWDeviceType type) {
  int err = 0;
  AVBufferRef* device_ref = NULL;
  AVHWDeviceContext* device_ctx;
  AVD3D11VADeviceContext* device_hwctx;

  HRESULT hr;
  UINT creation_flags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
  static D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_1 };

  D3D_FEATURE_LEVEL feature_levels_out;

  device_ref = av_hwdevice_ctx_alloc(type);
  if (!device_ref) {
    RTC_LOG(LS_ERROR) << "Failed to allocate d3d11va hw context.";
    err = -1;
    goto fail;
  }

  device_ctx = (AVHWDeviceContext*)device_ref->data;
  device_hwctx = (AVD3D11VADeviceContext*)device_ctx->hwctx;

  hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
      creation_flags, feature_levels, sizeof(feature_levels) / sizeof(feature_levels[0]), D3D11_SDK_VERSION,
      &device_hwctx->device, &feature_levels_out, &d3d11_device_context_);
  if (FAILED(hr)) {
    err = -1;
    goto fail;
  }

  d3d11_device_ = device_hwctx->device;

  if (d3d11_device_) {
    hr = d3d11_device_->QueryInterface(__uuidof(ID3D11VideoDevice),
        (void**)&d3d11_video_device_);
    if (FAILED(hr)) {
      err = -1;
      goto fail;
    }
  }
  if (d3d11_device_context_) {
    hr = d3d11_device_context_->QueryInterface(__uuidof(ID3D11VideoContext),
        (void**)&d3d11_video_context_);
    if (FAILED(hr)) {
      err = -1;
      goto fail;
    }
  }
  // Turn on multi-threading for the context
  {
    CComQIPtr<ID3D10Multithread> p_mt(device_hwctx->device);
    if (p_mt) {
      p_mt->SetMultithreadProtected(true);
    }
  }

  err = av_hwdevice_ctx_init(device_ref);
  if (err < 0) {
    RTC_LOG(LS_ERROR) << "Failed hwdevice_ctx_init.";
    goto fail;
  }

  hw_device_ctx = device_ref;
  ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
  return 0;

fail:
  av_buffer_unref(&device_ref);
  hw_device_ctx = nullptr;
  return err;
}

int H264DXVADecoderImpl::PrepareHwDecoder(webrtc::VideoCodecType codec_type) {
  int ret = 0;
  enum AVHWDeviceType type;

  type = av_hwdevice_find_type_by_name("d3d11va");
  if (type == AV_HWDEVICE_TYPE_NONE) {
    RTC_LOG(LS_ERROR) << "D3D11VA HW decoder not found. Avaiable decoders: ";
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
      RTC_LOG(LS_ERROR) << av_hwdevice_get_type_name(type);
      return -1;
    }
  }

  AVCodecID codec_id = codec_type == webrtc::VideoCodecType::kVideoCodecH264
                           ? AV_CODEC_ID_H264
                           : AV_CODEC_ID_HEVC;

  decoder = avcodec_find_decoder(codec_id);
  if (!decoder) {
    RTC_LOG(LS_ERROR) << "Decoder not found by avcodec_find_decoder.";
    return -1;
  }

  for (int i = 0;; i++) {
    const AVCodecHWConfig* config = avcodec_get_hw_config(decoder, i);
    if (!config) {
      RTC_LOG(LS_ERROR) << "Decoder " << decoder->name
        << " does not support device type: "
        << av_hwdevice_get_type_name(type);
      return -1;
    }
    if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
      config->device_type == type) {
      hw_pix_fmt = config->pix_fmt;
      break;
    }
  }

  if (!(decoder_ctx = avcodec_alloc_context3(decoder))) {
    RTC_LOG(LS_ERROR) << "Failed on alloc_context.";
    return -1;
  }

  decoder_ctx->get_format = get_hw_format;

  if (InitHwContext(decoder_ctx, type) < 0) {
    RTC_LOG(LS_ERROR) << "Failed InitHwContext.";
    return -1;
  }

  if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
    RTC_LOG(LS_ERROR) << "Failed to open decoder.";
    return -1;
  }
  return 0;
}

int32_t H264DXVADecoderImpl::InitDecode(const webrtc::VideoCodec* codec_settings,
                                    int32_t number_of_cores) {
  ReportInit();
  if (codec_settings &&
      (codec_settings->codecType != webrtc::kVideoCodecH264 
#ifdef WEBRTC_USE_H265
      && codec_settings->codecType != webrtc::kVideoCodecH265)
#endif
  ) {
    RTC_LOG(LS_ERROR) << "in H264DXVADecoderImpl: codec mismatch.";
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  // Release necessary in case of re-initializing.
  int32_t ret = Release();
  if (ret != WEBRTC_VIDEO_CODEC_OK) {
    ReportError();
    return ret;
  }

  ret = PrepareHwDecoder(codec_settings->codecType);
  if (ret < 0) {
    RTC_LOG(LS_ERROR) << "Failed to prepare the hw decoder.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264DXVADecoderImpl::Release() {
  // Do we need to flush the decoder? Currently we don't.
  if (decoder_ctx != nullptr) {
    avcodec_free_context(&decoder_ctx);
    decoder_ctx = nullptr;
  }
  if (hw_device_ctx != nullptr) {
    av_buffer_unref(&hw_device_ctx);
    hw_device_ctx = nullptr;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264DXVADecoderImpl::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback) {
  decoded_image_callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264DXVADecoderImpl::Decode(const webrtc::EncodedImage& input_image,
                                bool /*missing_frames*/,
                                int64_t /*render_time_ms*/) {
  if (!IsInitialized()) {
    ReportError();
    RTC_LOG(LS_ERROR) << "Decoder not initialized.";
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!decoded_image_callback_) {
    RTC_LOG(LS_WARNING)
        << "InitDecode() has been called, but a callback function "
           "has not been set with RegisterDecodeCompleteCallback()";
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!input_image.data() || !input_image.size()) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  AVFrame* frame = nullptr;
  AVPacket packet;
  av_init_packet(&packet);
  packet.data = input_image.mutable_data();

  if (input_image.size() >
      static_cast<size_t>(std::numeric_limits<int>::max())) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  packet.size = static_cast<int>(input_image.size());

  GetSideData(input_image.mutable_data(), input_image.size(), current_side_data_);
  if (current_side_data_.size() > 0) {
    side_data_list_[input_image.Timestamp()] = current_side_data_;
  }
  decoder_ctx->reordered_opaque = (int64_t)input_image.Timestamp();

  int result = avcodec_send_packet(decoder_ctx, &packet);
  if (result < 0) {
    RTC_LOG(LS_ERROR) << "Send packet returned error: " << result;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Consume buffer until we have a frame.
  while (true) {
    if (!(frame = av_frame_alloc())) {
      RTC_LOG(LS_ERROR) << "Failed to alloc frame for decoding.";
      goto fail;
    }

    result = avcodec_receive_frame(decoder_ctx, frame);
    if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
      av_frame_free(&frame);
      RTC_LOG(LS_VERBOSE) << "No frame. Return from decoder.";
      return WEBRTC_VIDEO_CODEC_OK;
    }
    else if (result < 0) {
      RTC_LOG(LS_ERROR) << "Error happend during decoding.";
      goto fail;
    }

    // We get one frame from the decoder.
    if (frame != nullptr && frame->format == hw_pix_fmt) {
      ID3D11Texture2D* texture = (ID3D11Texture2D*)frame->data[0];
      int index = (intptr_t)frame->data[1];
      D3D11_TEXTURE2D_DESC texture_desc;

      if (texture == nullptr) {
        RTC_LOG(LS_ERROR) << "Invalid output texture.";
        goto fail;
      }
      texture->GetDesc(&texture_desc);
      if (decoded_image_callback_) {
        if (!d3d11_device_ || !d3d11_video_device_ || !d3d11_video_context_) {
          RTC_LOG(LS_ERROR)
            << "Invalid d3d11 context to be passed to renderer.";
          goto fail;
        }

        uint32_t pts = static_cast<uint32_t>(frame->reordered_opaque);
        size_t side_data_size = 0;
        RtlZeroMemory(&surface_handle_->side_data[0],
                      OWT_ENCODED_IMAGE_SIDE_DATA_SIZE_MAX);
        if (side_data_list_.find(pts) != side_data_list_.end()) {
          side_data_size = side_data_list_[pts].size();
          for (int i = 0; i < side_data_size; i++) {
            surface_handle_->side_data[i] = side_data_list_[pts][i];
          }
          side_data_list_.erase(pts);
          if (side_data_list_.size() > kMaxSideDataListSize) {
            // If side_data_list_ grows too large, clear it.
            side_data_list_.clear();
          }
        }
        // Should we only send the d3d11device, and let renderer derive video
        // device and context by itself?
        surface_handle_->texture = texture;
        surface_handle_->d3d11_device = d3d11_device_.p;
        surface_handle_->d3d11_video_device = d3d11_video_device_.p;
        surface_handle_->context = d3d11_video_context_.p;
        surface_handle_->array_index = index;
        surface_handle_->side_data_size = side_data_size;
        rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
          new rtc::RefCountedObject<owt::base::NativeHandleBuffer>(
          (void*)surface_handle_.get(), texture_desc.Width,
            texture_desc.Height);
        webrtc::VideoFrame decoded_frame(buffer, input_image.Timestamp(), 0,
          webrtc::kVideoRotation_0);
        decoded_frame.set_ntp_time_ms(input_image.ntp_time_ms_);
        decoded_frame.set_timestamp(input_image.Timestamp());
        decoded_image_callback_->Decoded(decoded_frame);

      }
      av_frame_free(&frame);
    }
  }
  return WEBRTC_VIDEO_CODEC_OK;

fail:
  av_frame_free(&frame);
  return WEBRTC_VIDEO_CODEC_ERROR;
}

// Helper function to extract the prefix-SEI.
int64_t H264DXVADecoderImpl::GetSideData(const uint8_t* frame_data,
  size_t frame_size, std::vector<uint8_t>& side_data) {
  side_data.clear();
  if (frame_size < 24)  // with prefix-frame-num sei, frame size needs to be at least 24 bytes.
    goto failed;

  const uint8_t* head = frame_data;
  unsigned int payload_size = 0;

  if (head[0] != 0 || head[1] != 0 || head[2] != 0 || head[3] != 1) {
    goto failed;
  }
  if ((head[4] & 0x1f) == 0x06) {
    if (head[5] == 0x05) { // user data unregistered.
      payload_size = head[6];
      if (payload_size > frame_size - 4 - 4 || payload_size < 17) //. 4-byte start code + 4 byte NAL HDR/Payload Type/Size/RBSP
        goto failed;
      for (int i = 7; i < 23; i++) {
        if (head[i] != frame_number_sei_guid[i - 7]) {
          goto failed;
        }
      }
      // Read the entire payload
      for (unsigned int i = 0; i < payload_size - 16; i++)
        side_data.push_back(head[i + 23]);
      return payload_size;
    }
  }
failed:
  return -1;
}

const char* H264DXVADecoderImpl::ImplementationName() const {
  return "OWTD3D11VA";
}

bool H264DXVADecoderImpl::IsInitialized() const {
  return decoder_ctx != nullptr;
}

void H264DXVADecoderImpl::ReportInit() {
  if (has_reported_init_)
    return;
  RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.H264DXVADecoderImpl.Event",
                            kH264DecoderEventInit,
                            kH264DecoderEventMax);
  has_reported_init_ = true;
}

void H264DXVADecoderImpl::ReportError() {
  if (has_reported_error_)
    return;
  RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.H264DXVADecoderImpl.Event",
                            kH264DecoderEventError,
                            kH264DecoderEventMax);
  has_reported_error_ = true;
}

std::unique_ptr<H264DXVADecoderImpl> H264DXVADecoderImpl::Create(
    cricket::VideoCodec format) {
  return absl::make_unique<H264DXVADecoderImpl>(nullptr);
}

}  // namespace base
}  // namespace owt
