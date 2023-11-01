// Copyright (C) <2023> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/ffmpeg_decoder_impl.h"

#include <algorithm>
#include <limits>
#include <memory>

extern "C" {
#include "third_party/ffmpeg/libavcodec/avcodec.h"
#include "third_party/ffmpeg/libavformat/avformat.h"
#include "third_party/ffmpeg/libavutil/imgutils.h"
}  // extern "C"

#include "api/scoped_refptr.h"
#include "api/video/color_space.h"
#include "api/video/i420_buffer.h"
#include "api/video/i444_buffer.h"
#include "api/video/render_resolution.h"
#include "api/video/video_frame.h"
#include "common_video/include/video_frame_buffer.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "system_wrappers/include/metrics.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "webrtc/system_wrappers/include/clock.h"

#include "talk/owt/sdk/base/nativehandlebuffer.h"

namespace owt {
namespace base {

namespace {

constexpr std::array<AVPixelFormat, 2> kPixelFormatsSupported = {
    AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV444P};
constexpr size_t kYPlaneIndex = 0;
constexpr size_t kUPlaneIndex = 1;
constexpr size_t kVPlaneIndex = 2;

struct ScopedPtrAVFreePacket {
  void operator()(AVPacket* packet) { av_packet_free(&packet); }
};
typedef std::unique_ptr<AVPacket, ScopedPtrAVFreePacket> ScopedAVPacket;

ScopedAVPacket MakeScopedAVPacket() {
  ScopedAVPacket packet(av_packet_alloc());
  return packet;
}

}  // namespace

int FFMpegDecoderImpl::AVGetBuffer2(AVCodecContext* context,
                                    AVFrame* av_frame,
                                    int flags) {
  // Set in `Configure`.
  FFMpegDecoderImpl* decoder = static_cast<FFMpegDecoderImpl*>(context->opaque);
  // DCHECK values set in `Configure`.
  RTC_DCHECK(decoder);
  // Necessary capability to be allowed to provide our own buffers.
  RTC_DCHECK(context->codec->capabilities | AV_CODEC_CAP_DR1);

  auto pixelFormatSupported = std::find_if(
      kPixelFormatsSupported.begin(), kPixelFormatsSupported.end(),
      [context](AVPixelFormat format) { return context->pix_fmt == format; });

  RTC_CHECK(pixelFormatSupported != kPixelFormatsSupported.end());

  // `av_frame->width` and `av_frame->height` are set by FFmpeg. These are the
  // actual image's dimensions and may be different from `context->width` and
  // `context->coded_width` due to reordering.
  int width = av_frame->width;
  int height = av_frame->height;
  // See `lowres`, if used the decoder scales the image by 1/2^(lowres). This
  // has implications on which resolutions are valid, but we don't use it.
  RTC_CHECK_EQ(context->lowres, 0);
  // Adjust the `width` and `height` to values acceptable by the decoder.
  // Without this, FFmpeg may overflow the buffer. If modified, `width` and/or
  // `height` are larger than the actual image and the image has to be cropped
  // (top-left corner) after decoding to avoid visible borders to the right and
  // bottom of the actual image.
  avcodec_align_dimensions(context, &width, &height);

  RTC_CHECK_GE(width, 0);
  RTC_CHECK_GE(height, 0);
  int ret = av_image_check_size(static_cast<unsigned int>(width),
                                static_cast<unsigned int>(height), 0, nullptr);
  if (ret < 0) {
    RTC_LOG(LS_ERROR) << "Invalid picture size " << width << "x" << height;
    decoder->ReportError();
    return ret;
  }

  // The video frame is stored in `frame_buffer`. `av_frame` is FFmpeg's version
  // of a video frame and will be set up to reference `frame_buffer`'s data.
  rtc::scoped_refptr<webrtc::PlanarYuvBuffer> frame_buffer;
  rtc::scoped_refptr<webrtc::I444Buffer> i444_buffer;
  rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer;

  // TODO:We only support 8bpp formats. If 10b/12b-444 is going to be supported,
  // will need to update this.
  int bytes_per_pixel = 1;
  switch (context->pix_fmt) {
    case AV_PIX_FMT_YUV420P:
      i420_buffer =
          decoder->ffmpeg_buffer_pool_.CreateI420Buffer(width, height);
      // Set `av_frame` members as required by FFmpeg.
      av_frame->data[kYPlaneIndex] = i420_buffer->MutableDataY();
      av_frame->linesize[kYPlaneIndex] = i420_buffer->StrideY();
      av_frame->data[kUPlaneIndex] = i420_buffer->MutableDataU();
      av_frame->linesize[kUPlaneIndex] = i420_buffer->StrideU();
      av_frame->data[kVPlaneIndex] = i420_buffer->MutableDataV();
      av_frame->linesize[kVPlaneIndex] = i420_buffer->StrideV();
      RTC_DCHECK_EQ(av_frame->extended_data, av_frame->data);
      frame_buffer = i420_buffer;
      break;
    case AV_PIX_FMT_YUV444P:
      i444_buffer =
          decoder->ffmpeg_buffer_pool_.CreateI444Buffer(width, height);
      // Set `av_frame` members as required by FFmpeg.
      av_frame->data[kYPlaneIndex] = i444_buffer->MutableDataY();
      av_frame->linesize[kYPlaneIndex] = i444_buffer->StrideY();
      av_frame->data[kUPlaneIndex] = i444_buffer->MutableDataU();
      av_frame->linesize[kUPlaneIndex] = i444_buffer->StrideU();
      av_frame->data[kVPlaneIndex] = i444_buffer->MutableDataV();
      av_frame->linesize[kVPlaneIndex] = i444_buffer->StrideV();
      frame_buffer = i444_buffer;
      break;
    default:
      RTC_LOG(LS_ERROR) << "Unsupported buffer type " << context->pix_fmt
                        << ". Check supported supported pixel formats!";
      decoder->ReportError();
      return -1;
  }

  int y_size = width * height * bytes_per_pixel;
  int uv_size = frame_buffer->ChromaWidth() * frame_buffer->ChromaHeight() *
                bytes_per_pixel;
  // DCHECK that we have a continuous buffer as is required.
  RTC_DCHECK_EQ(av_frame->data[kUPlaneIndex],
                av_frame->data[kYPlaneIndex] + y_size);
  RTC_DCHECK_EQ(av_frame->data[kVPlaneIndex],
                av_frame->data[kUPlaneIndex] + uv_size);
  int total_size = y_size + 2 * uv_size;

  av_frame->format = context->pix_fmt;
  av_frame->reordered_opaque = context->reordered_opaque;

  // Create a VideoFrame object, to keep a reference to the buffer.
  av_frame->buf[0] = av_buffer_create(
      av_frame->data[kYPlaneIndex], total_size, AVFreeBuffer2,
      static_cast<void*>(std::make_unique<webrtc::VideoFrame>(
                             webrtc::VideoFrame::Builder()
                                 .set_video_frame_buffer(frame_buffer)
                                 .set_rotation(webrtc::kVideoRotation_0)
                                 .set_timestamp_us(0)
                                 .build())
                             .release()),
      0);
  RTC_CHECK(av_frame->buf[0]);
  return 0;
}

void FFMpegDecoderImpl::AVFreeBuffer2(void* opaque, uint8_t* data) {
  // The buffer pool recycles the buffer used by `video_frame` when there are no
  // more references to it. `video_frame` is a thin buffer holder and is not
  // recycled.
  webrtc::VideoFrame* video_frame = static_cast<webrtc::VideoFrame*>(opaque);
  delete video_frame;
}

FFMpegDecoderImpl::FFMpegDecoderImpl()
    : ffmpeg_buffer_pool_(true),
      decoded_image_callback_(nullptr),
      has_reported_init_(false),
      has_reported_error_(false),
      clock_(webrtc::Clock::GetRealTimeClock())  {
  surface_handle_.reset(new D3D11VAHandle());
}

FFMpegDecoderImpl::~FFMpegDecoderImpl() {
  Release();
}

bool FFMpegDecoderImpl::Configure(
    const webrtc::VideoDecoder::Settings& settings) {
  ReportInit();
  if (settings.codec_type() != webrtc::kVideoCodecH265) {
    RTC_LOG(LS_ERROR) << "FFmpegDecoder only supports H265 codec.";
    ReportError();
    return false;
  }

  // Release necessary in case of re-initializing.
  int32_t ret = Release();
  if (ret != WEBRTC_VIDEO_CODEC_OK) {
    ReportError();
    return false;
  }
  RTC_DCHECK(!av_context_);

  // Initialize AVCodecContext.
  av_context_.reset(avcodec_alloc_context3(nullptr));

  av_context_->codec_type = AVMEDIA_TYPE_VIDEO;
  av_context_->codec_id = AV_CODEC_ID_H265;
  const webrtc::RenderResolution& resolution = settings.max_render_resolution();
  if (resolution.Valid()) {
    av_context_->coded_width = resolution.Width();
    av_context_->coded_height = resolution.Height();
  }
  av_context_->extradata = nullptr;
  av_context_->extradata_size = 0;

  // If this is ever increased, look at `av_context_->thread_safe_callbacks` and
  // make it possible to disable the thread checker in the frame buffer pool.
  av_context_->thread_count = 1;
  av_context_->thread_type = FF_THREAD_SLICE;

  // Function used by FFmpeg to get buffers to store decoded frames in.
  av_context_->get_buffer2 = AVGetBuffer2;
  // `get_buffer2` is called with the context, there `opaque` can be used to get
  // a pointer `this`.
  av_context_->opaque = this;

  const AVCodec* codec = avcodec_find_decoder(av_context_->codec_id);
  if (!codec) {
    // This is an indication that FFmpeg has not been initialized or it has not
    // been compiled/initialized with the correct set of codecs.
    RTC_LOG(LS_ERROR) << "FFmpeg H.265 decoder not found.";
    Release();
    ReportError();
    return false;
  }
  int res = avcodec_open2(av_context_.get(), codec, nullptr);
  if (res < 0) {
    RTC_LOG(LS_ERROR) << "avcodec_open2 error: " << res;
    Release();
    ReportError();
    return false;
  }

  av_frame_.reset(av_frame_alloc());

  if (absl::optional<int> buffer_pool_size = settings.buffer_pool_size()) {
    if (!ffmpeg_buffer_pool_.Resize(*buffer_pool_size)) {
      return false;
    }
  }
  HRESULT hr;
  UINT creation_flags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
  m_padapter_ = nullptr;
  static D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_1 };

  D3D_FEATURE_LEVEL feature_levels_out;

  hr = D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creation_flags, feature_levels,
      sizeof(feature_levels) / sizeof(feature_levels[0]), D3D11_SDK_VERSION,
      &d3d11_device_, &feature_levels_out, &d3d11_device_context_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create D3D11 device for decode output.";
    return false;
  }

  if (d3d11_device_) {
    hr = d3d11_device_->QueryInterface(__uuidof(ID3D11VideoDevice),
        (void**)&d3d11_video_device_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get video device from D3D11 device.";
      return false;
    }
  }
  if (d3d11_device_context_) {
    hr = d3d11_device_context_->QueryInterface(__uuidof(ID3D11VideoContext),
        (void**)&d3d11_video_context_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get video context from D3D11 device context.";
      return false;
    }
  }
  return true;
}

int32_t FFMpegDecoderImpl::Release() {
  av_context_.reset();
  av_frame_.reset();
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t FFMpegDecoderImpl::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback) {
  decoded_image_callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

void FFMpegDecoderImpl::CreateStagingTextureIfNeeded(int width, int height) {
  HRESULT hr = S_OK;
  D3D11_TEXTURE2D_DESC desc = {0};
  if (staging_texture_) {
    D3D11_TEXTURE2D_DESC desc = {0};
    staging_texture_->GetDesc(&desc);
    if (desc.Width != (unsigned int)width ||
        desc.Height != (unsigned int)height) {
      staging_texture_.Release();
    } else {
      goto output;
    }
  }

  desc.Width = (unsigned int)width;
  desc.Height = (unsigned int)height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
  desc.MiscFlags = 0;
  desc.BindFlags = 0;

  d3d11_device_->CreateTexture2D(&desc, nullptr, &staging_texture_);

output:
  D3D11_TEXTURE2D_DESC output_desc = {0};
  staging_texture_->GetDesc(&output_desc);
  if (output_texture_) {
    D3D11_TEXTURE2D_DESC orig_desc = {0};
    output_texture_->GetDesc(&orig_desc);
    if (orig_desc.Width != (unsigned int)width ||
        orig_desc.Height != (unsigned int)height) {
      output_texture_.Release();
    } else {
      return;
    }
  }
  output_desc.Usage = D3D11_USAGE_DEFAULT;
  output_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
  output_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
  d3d11_device_->CreateTexture2D(&output_desc, nullptr, &output_texture_);

  return;
}

int32_t FFMpegDecoderImpl::Decode(const webrtc::EncodedImage& input_image,
                                  bool /*missing_frames*/,
                                  int64_t /*render_time_ms*/) {
  if (!IsInitialized()) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!decoded_image_callback_) {
    RTC_LOG(LS_WARNING)
        << "Configure() has been called, but a callback function "
           "has not been set with RegisterDecodeCompleteCallback()";
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!input_image.data() || !input_image.size()) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  int64_t decode_start_time = clock_->CurrentTime().ms_or(0);

  ScopedAVPacket packet = MakeScopedAVPacket();
  if (!packet) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  // packet.data has a non-const type, but isn't modified by
  // avcodec_send_packet.
  packet->data = const_cast<uint8_t*>(input_image.data());
  if (input_image.size() >
      static_cast<size_t>(std::numeric_limits<int>::max())) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  packet->size = static_cast<int>(input_image.size());
  int64_t frame_timestamp_us = input_image.ntp_time_ms_ * 1000;  // ms -> μs
  av_context_->reordered_opaque = frame_timestamp_us;

  int result = avcodec_send_packet(av_context_.get(), packet.get());

  if (result < 0) {
    RTC_LOG(LS_ERROR) << "avcodec_send_packet error: " << result;
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  result = avcodec_receive_frame(av_context_.get(), av_frame_.get());
  if (result < 0) {
    RTC_LOG(LS_ERROR) << "avcodec_receive_frame error: " << result;
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // We don't expect reordering. Decoded frame timestamp should match
  // the input one.
  RTC_DCHECK_EQ(av_frame_->reordered_opaque, frame_timestamp_us);

  // Maybe it is possible to get QP directly from FFmpeg.
  h265_bitstream_parser_.ParseBitstream(input_image);
  absl::optional<int> qp = h265_bitstream_parser_.GetLastSliceQp();

  // Obtain the `video_frame` containing the decoded image.
  webrtc::VideoFrame* input_frame =
      static_cast<webrtc::VideoFrame*>(av_buffer_get_opaque(av_frame_->buf[0]));
  RTC_DCHECK(input_frame);
  rtc::scoped_refptr<webrtc::VideoFrameBuffer> frame_buffer =
      input_frame->video_frame_buffer();

  // Instantiate Planar YUV buffer according to video frame buffer type
  const webrtc::PlanarYuvBuffer* planar_yuv_buffer = nullptr;
  const webrtc::PlanarYuv8Buffer* planar_yuv8_buffer = nullptr;
  const webrtc::PlanarYuv16BBuffer* planar_yuv16_buffer = nullptr;
  webrtc::VideoFrameBuffer::Type video_frame_buffer_type = frame_buffer->type();
  switch (video_frame_buffer_type) {
    case webrtc::VideoFrameBuffer::Type::kI420:
      planar_yuv_buffer = frame_buffer->GetI420();
      planar_yuv8_buffer =
          reinterpret_cast<const webrtc::PlanarYuv8Buffer*>(planar_yuv_buffer);
      break;
    case webrtc::VideoFrameBuffer::Type::kI444:
      planar_yuv_buffer = frame_buffer->GetI444();
      planar_yuv8_buffer =
          reinterpret_cast<const webrtc::PlanarYuv8Buffer*>(planar_yuv_buffer);
      break;
    default:
      RTC_LOG(LS_ERROR) << "frame_buffer type: "
                        << static_cast<int32_t>(video_frame_buffer_type)
                        << " is not supported!";
      ReportError();
      return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // When needed, FFmpeg applies cropping by moving plane pointers and adjusting
  // frame width/height. Ensure that cropped buffers lie within the allocated
  // memory.
  RTC_DCHECK_LE(av_frame_->width, planar_yuv_buffer->width());
  RTC_DCHECK_LE(av_frame_->height, planar_yuv_buffer->height());
  switch (video_frame_buffer_type) {
    case webrtc::VideoFrameBuffer::Type::kI420:
    case webrtc::VideoFrameBuffer::Type::kI444: {
      RTC_DCHECK_GE(av_frame_->data[kYPlaneIndex], planar_yuv8_buffer->DataY());
      RTC_DCHECK_LE(
          av_frame_->data[kYPlaneIndex] +
              av_frame_->linesize[kYPlaneIndex] * av_frame_->height,
          planar_yuv8_buffer->DataY() +
              planar_yuv8_buffer->StrideY() * planar_yuv8_buffer->height());
      RTC_DCHECK_GE(av_frame_->data[kUPlaneIndex], planar_yuv8_buffer->DataU());
      RTC_DCHECK_LE(
          av_frame_->data[kUPlaneIndex] +
              av_frame_->linesize[kUPlaneIndex] *
                  planar_yuv8_buffer->ChromaHeight(),
          planar_yuv8_buffer->DataU() + planar_yuv8_buffer->StrideU() *
                                            planar_yuv8_buffer->ChromaHeight());
      RTC_DCHECK_GE(av_frame_->data[kVPlaneIndex], planar_yuv8_buffer->DataV());
      RTC_DCHECK_LE(
          av_frame_->data[kVPlaneIndex] +
              av_frame_->linesize[kVPlaneIndex] *
                  planar_yuv8_buffer->ChromaHeight(),
          planar_yuv8_buffer->DataV() + planar_yuv8_buffer->StrideV() *
                                            planar_yuv8_buffer->ChromaHeight());
      break;
    }
    default:
      RTC_LOG(LS_ERROR) << "frame_buffer type: "
                        << static_cast<int32_t>(video_frame_buffer_type)
                        << " is not supported!";
      ReportError();
      return WEBRTC_VIDEO_CODEC_ERROR;
  }

  CreateStagingTextureIfNeeded(av_frame_->width, av_frame_->height);
  D3D11_MAPPED_SUBRESOURCE sub_resource = {0};
  HRESULT hr = d3d11_device_context_->Map(staging_texture_, 0,  D3D11_MAP_READ_WRITE, 0, &sub_resource);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to map texture.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  switch (video_frame_buffer_type) {
    case webrtc::VideoFrameBuffer::Type::kI444:
      libyuv::I444ToARGB(av_frame_->data[kYPlaneIndex], av_frame_->linesize[kYPlaneIndex],
                     av_frame_->data[kUPlaneIndex], av_frame_->linesize[kUPlaneIndex],
                     av_frame_->data[kVPlaneIndex], av_frame_->linesize[kVPlaneIndex],
                     static_cast<uint8_t*>(sub_resource.pData),
                     sub_resource.RowPitch,
                     av_frame_->width,
                     av_frame_->height);
      break;
    case webrtc::VideoFrameBuffer::Type::kI420:
      libyuv::I420ToARGB(av_frame_->data[kYPlaneIndex], av_frame_->linesize[kYPlaneIndex],
                     av_frame_->data[kUPlaneIndex], av_frame_->linesize[kUPlaneIndex],
                     av_frame_->data[kVPlaneIndex], av_frame_->linesize[kVPlaneIndex],
                     static_cast<uint8_t*>(sub_resource.pData),
                     sub_resource.RowPitch,
                     av_frame_->width,
                     av_frame_->height);
      break;
    default:
      RTC_LOG(LS_ERROR) << "frame_buffer type: "
                        << static_cast<int32_t>(video_frame_buffer_type)
                        << " conversion is not supported!";
      ReportError();
      return WEBRTC_VIDEO_CODEC_ERROR;
  }
  d3d11_device_context_->Unmap(staging_texture_, 0);

  d3d11_device_context_->CopyResource(output_texture_, staging_texture_);

  surface_handle_->texture = output_texture_.p;
  surface_handle_->d3d11_device = d3d11_device_.p;
  surface_handle_->d3d11_video_device = d3d11_video_device_.p;
  surface_handle_->context = d3d11_video_context_.p;
  surface_handle_->array_index = 0;
  surface_handle_->side_data_size = 0;
  surface_handle_->cursor_data_size = 0;
  surface_handle_->decode_start = decode_start_time;
  surface_handle_->decode_end = clock_->CurrentTime().ms_or(0);
  surface_handle_->start_duration =
            input_image.bwe_stats_.start_duration_;
  surface_handle_->last_duration = input_image.bwe_stats_.last_duration_;
  surface_handle_->packet_loss = input_image.bwe_stats_.packets_lost_;
  surface_handle_->frame_size = input_image.size();
  rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
            rtc::make_ref_counted<owt::base::NativeHandleBuffer>(
                (void*)surface_handle_.get(), av_frame_->width, av_frame_->height);
  webrtc::VideoFrame decoded_frame(buffer, input_image.Timestamp(), 0,
          webrtc::kVideoRotation_0);
  decoded_frame.set_ntp_time_ms(input_image.ntp_time_ms_);
  decoded_frame.set_timestamp(input_image.Timestamp());
  decoded_image_callback_->Decoded(decoded_frame, absl::nullopt, qp);

  // Stop referencing it, possibly freeing `input_frame`.
  av_frame_unref(av_frame_.get());
  input_frame = nullptr;

  return WEBRTC_VIDEO_CODEC_OK;
}

const char* FFMpegDecoderImpl::ImplementationName() const {
  return "FFmpegDecoder";
}

bool FFMpegDecoderImpl::IsInitialized() const {
  return av_context_ != nullptr;
}

void FFMpegDecoderImpl::ReportInit() {
  if (has_reported_init_)
    return;
  has_reported_init_ = true;
}

void FFMpegDecoderImpl::ReportError() {
  if (has_reported_error_)
    return;
  has_reported_error_ = true;
}

}  // namespace base
}  // namespace owt
