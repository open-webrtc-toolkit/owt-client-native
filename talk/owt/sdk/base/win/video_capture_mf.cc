// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/win/video_capture_mf.h"

#include <atlbase.h>
#include <atlcom.h>
#include <system_error>
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "third_party/libyuv/include/libyuv.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/api/video/i420_buffer.h"
#include "webrtc/api/video/video_frame_buffer.h"
#include "webrtc/api/video/nv12_buffer.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/modules/video_capture/video_capture_config.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/string_utils.h"
#include "webrtc/rtc_base/time_utils.h"


namespace webrtc {
namespace videocapturemodule {

std::unique_ptr<VideoCaptureMF> VideoCaptureMF::Create(
      const char* device_id, int32_t width,
      int32_t height, int32_t fps) {
  if (device_id == nullptr) {
    return nullptr;
  }

  std::unique_ptr<VideoCaptureMF> capturer;
  capturer.reset(new VideoCaptureMF());
  if (!capturer->Init(device_id, width, height, fps)) {
    return nullptr;
  }

  return capturer;
}

VideoCaptureMF::VideoCaptureMF()
    : reader_(nullptr)
    , requested_capability_()
    , output_capability_()
    , ref_count_(1)
    , manager_(nullptr) {
  surface_handle_.reset(new owt::base::D3D11ImageHandle());
}

VideoCaptureMF::~VideoCaptureMF() {
  capture_started_ = false;
}

bool VideoCaptureMF::Init(const char* device_unique_id_utf8, int32_t width,
                          int32_t height, int32_t fps) {
  // Max Count will be kVideoCaptureUniqueNameLength(Max UniqueName length)
  const int32_t name_length = (int32_t)strnlen_s((char*)device_unique_id_utf8, kVideoCaptureDeviceNameLength);
  if (name_length > kVideoCaptureUniqueNameLength)
    return false;

  // Store the device name
  device_unique_id_.reset(new (std::nothrow) char[name_length + 1]);
  if (!device_unique_id_) {
    return false;
  }
  memcpy_s(device_unique_id_.get(), name_length + 1, device_unique_id_utf8,
           name_length + 1);

  if (info_.Init() != 0) {
    return false;
  }

  // Get the Media Source from device info.
  CComPtr<IMFMediaSource> source(info_.GetCaptureSource(device_unique_id_utf8));
  if (!source) {
    return false;
  }

  // Set the requested capabality
  requested_capability_.width = width;
  requested_capability_.height = height;
  requested_capability_.maxFPS = fps;
  requested_capability_.videoType = VideoType::kNV12;

  // Get the best matching capability
  VideoCaptureCapability capability;
  int32_t capability_index;

  // Store the new requested size
  // Match the requested capability with the supported.
  if ((capability_index = info_.GetBestMatchedCapability(
           device_unique_id_.get(), requested_capability_, capability)) < 0) {
    return false;
  }

  // Reduce the frame rate if possible.
  if (capability.maxFPS > requested_capability_.maxFPS) {
    capability.maxFPS = requested_capability_.maxFPS;
  } else if (capability.maxFPS <= 0) {
    capability.maxFPS = 30;
  }

  // Store the capability that is set to capture device
  output_capability_ = capability;
  RTC_LOG(LS_INFO) << "BestCapability Selected: width:"
                   << capability.width << ", height:" << capability.height
                   << ", FPS:" << capability.maxFPS
                   << ", videoType:" << capability.videoType;

  HRESULT hr = S_OK;

  CComPtr<IMFMediaSourceEx> media_source_ex = nullptr;
  if (output_capability_.videoType == webrtc::VideoType::kNV12) {
    // Sharing the buffer is only when the camera output is NV12
    manager_ = new rtc::RefCountedObject<owt::base::D3D11Manager>();
    if (!manager_->Init()) {
      RTC_LOG(LS_ERROR) << "Failed to initialize D3dD11 Manager.";
      return false;
    }
    hr = source->QueryInterface(&media_source_ex);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to query IMFMediaSourceEx:"
                        << std::system_category().message(hr);
      manager_.release();
      manager_ = nullptr;
      // If IMFMediaSourceEx is not available, then we will directly use
      // System memory to receive the frames.
    }
  }
  if (media_source_ex && manager_ &&
      FAILED(media_source_ex->SetD3DManager(manager_->GetManager()))) {
    // If SetD3DManager fails means that media source does not support source-level attributes,
    // trying to register for system memory buffers from MF.
    RTC_LOG(LS_ERROR) << "SetD3DManager failed:"
                      << std::system_category().message(hr);
    manager_->Release();
    manager_ = nullptr;
    media_source_ex.Release();
    media_source_ex = nullptr;
  }

  CComPtr<IMFAttributes> attributes = nullptr;
  hr = MFCreateAttributes(&attributes, 1);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "MFCreateAttributes failed:"
                      << std::system_category().message(hr);
    return false;
  }

  // Set the Async Callback attribute on the store
  hr = attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR)
        << "VideoCapturer: Setting Async Callback attribute failed:"
        << std::system_category().message(hr);
    return false;
  }

  hr = MFCreateSourceReaderFromMediaSource(source, attributes, &reader_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Creating IMFSourceReader failed:"
                      << std::system_category().message(hr);
    source->Shutdown();
    return false;
  }

  // Set format to capture device
  CComPtr<IMFMediaType> media_type = nullptr;
  hr = reader_->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                   capability_index, &media_type);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "GetNativeMediaType failed:"
                      << std::system_category().message(hr);
    return false;
  }

  hr = reader_->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                    nullptr, media_type);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "SetCurrentMediaType failed:"
                      << std::system_category().message(hr);
    return false;
  }

  return true;
}

bool VideoCaptureMF::StartCapture(int32_t width, int32_t height, int32_t fps) {
  if (capture_started_) {
    return true;
  }

  if (!reader_) {
    return false;
  }

  webrtc::MutexLock lock(&mutex_);
  VideoCaptureCapability capability;
  capability.width = width;
  capability.height = height;
  capability.maxFPS = fps;
  capability.videoType = VideoType::kNV12;

  if (FAILED(SetCameraOutput(capability))) {
    return false;
  }

  // As Source Reader is running in async mode, need to request
  // for first sample from the source.
  HRESULT hr = reader_->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0,
                                   nullptr, nullptr, nullptr, nullptr);
  capture_started_ = SUCCEEDED(hr);

  return SUCCEEDED(hr);
}

bool VideoCaptureMF::StopCapture() {
  if (!capture_started_) {
    return true;
  }
  webrtc::MutexLock lock(&mutex_);

  if (reader_) {
    HRESULT hr = reader_->Flush(MF_SOURCE_READER_ALL_STREAMS);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "SourceReader flush failed:"
                        << std::system_category().message(hr);
    }
  }

  capture_started_ = false;
  return true;
}

// VideoSourceInterface Methods
void VideoCaptureMF::AddOrUpdateSink(rtc::VideoSinkInterface<VideoFrame>* sink,
    const rtc::VideoSinkWants& wants) {
  webrtc::MutexLock lock(&mutex_);
  broadcaster_.AddOrUpdateSink(sink, wants);
}

void VideoCaptureMF::RemoveSink(rtc::VideoSinkInterface<VideoFrame>* sink)
{
  webrtc::MutexLock lock(&mutex_);
  broadcaster_.RemoveSink(sink);
}

// IMFSourceReaderCallback methods
STDMETHODIMP VideoCaptureMF::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
                                          DWORD dwStreamFlags, LONGLONG llTimestamp,
                                          IMFSample* sample) {
  if (!capture_started_)
    return S_OK;

  if (FAILED(hrStatus)) {
    RTC_LOG(LS_ERROR) << "OnReadSample reports error:"
                      << std::system_category().message(hrStatus);
    // The source reader ignores the return value, so returning OK
    return S_OK;
  }

  HRESULT hr = S_OK;
  webrtc::MutexLock lock(&mutex_);

  if (sample) {
    IMFMediaBuffer* buffer;

    if (manager_) {
      hr = sample->GetBufferByIndex(0, &buffer);
    } else {
      hr = sample->ConvertToContiguousBuffer(&buffer);
    }
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to retrieve buffer from IMFSample:"
                        << std::system_category().message(hr);
      // The source reader ignores the return value, so returning OK
      return S_OK;
    }
    IncomingFrame(buffer, llTimestamp);
  }

  // Request for next frame
  if (SUCCEEDED(hr)) {
    hr = reader_->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
  }

  return S_OK;
}

void VideoCaptureMF::IncomingFrame(IMFMediaBuffer*& frame, int64_t capture_time) {
  if (frame == nullptr) {
    return;
  }

  DWORD frame_length = 0;

  const int32_t width = output_capability_.width;
  const int32_t height = output_capability_.height;

  rtc::scoped_refptr<VideoFrameBuffer> buffer;

  HRESULT hr = S_OK;
  ID3D11Texture2D* texture = nullptr;
  if (manager_) {
    CComPtr<IMFDXGIBuffer> dxgi_buffer;
    hr = frame->QueryInterface(&dxgi_buffer);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Query DxgiBuffer failed:"
                        << std::system_category().message(hr);
      return;
    }

    // Retrieves the texture from dxgi buffer
    if (FAILED(dxgi_buffer->GetResource(IID_PPV_ARGS(&texture)))) {
      RTC_LOG(LS_ERROR) << "Retreiving the texture from dxgi buffer failed:"
                        << std::system_category().message(hr);
      return;
    }
    surface_handle_->d3d11_device = manager_->GetDevice();
    surface_handle_->texture = std::move(texture);
    surface_handle_->texture_array_index = 0;
    rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
        new rtc::RefCountedObject<owt::base::NativeHandleBuffer>(
            (void*)surface_handle_.get(), width, height);
    SAFE_RELEASE(frame);
  } else {
    if (FAILED(frame->GetCurrentLength(&frame_length))) {
      return;
    }

    if (output_capability_.videoType != VideoType::kMJPEG &&
        CalcBufferSize(output_capability_.videoType, width, abs(height)) != frame_length) {
      RTC_LOG(LS_ERROR) << "Wrong Incoming frame length.";
      return;
    }
    if (output_capability_.videoType == VideoType::kNV12) {
      buffer = webrtc::NV12Buffer::Create(width, height);
    } else {
      int stride_y = width;
      int stride_uv = (width + 1) / 2;
      int target_width = width;
      int target_height = abs(height);
      BYTE* data = nullptr;

      if (FAILED(frame->Lock(&data, nullptr, nullptr))) {
        SAFE_RELEASE(frame);
        return;
      }

      // Setting absolute height (in case it was negative).
      // In Windows, the image starts bottom left, instead of top left.
      // Setting a negative source height, inverts the image (within LibYuv).
      rtc::scoped_refptr<I420Buffer> i420_buffer = I420Buffer::Create(
          target_width, target_height, stride_y, stride_uv, stride_uv);

      int ret = libyuv::ConvertToI420(
          data, frame_length, i420_buffer.get()->MutableDataY(),
          i420_buffer.get()->StrideY(), i420_buffer.get()->MutableDataU(),
          i420_buffer.get()->StrideU(), i420_buffer.get()->MutableDataV(),
          i420_buffer.get()->StrideV(), 0, 0,  // No Cropping
          width, height, target_width, target_height, libyuv::kRotate0,
          ConvertVideoType(output_capability_.videoType));
      if (ret < 0) {
        RTC_LOG(LS_ERROR)
            << "Failed to convert capture frame from type "
            << static_cast<int>(output_capability_.videoType) << "to I420.";
        i420_buffer.release();
        SAFE_RELEASE(frame);
        return;
      }
      buffer = std::move(i420_buffer);
      if (FAILED(frame->Unlock())) {
        buffer.release();
        SAFE_RELEASE(frame);
        return;
      }
      SAFE_RELEASE(frame);
    }
  }

  VideoFrame captured_frame = VideoFrame::Builder()
                                .set_video_frame_buffer(buffer)
                                .set_timestamp_rtp(0)
                                .set_timestamp_ms(rtc::TimeMillis())
                                .set_rotation(kVideoRotation_0)
                                .build();

  // TODO(jianlin): The timestamp returned by callback is not the same clock source
  // of webrtc. Should we set it?
  //captured_frame.set_ntp_time_ms(capture_time);

  broadcaster_.OnFrame(captured_frame);

  return;
}

HRESULT VideoCaptureMF::SetCameraOutput(
    const VideoCaptureCapability& requested_capability) {
  if (requested_capability_ == requested_capability) {
    return S_OK;
  }

  // Get the best matching capability
  VideoCaptureCapability capability;
  int32_t capabilityIndex;
  std::string format;

  // Store the new requested size
  requested_capability_ = requested_capability;
  // Match the requested capability with the supported.
  if ((capabilityIndex = info_.GetBestMatchedCapability(
           device_unique_id_.get(), requested_capability_, capability)) < 0) {
    return S_FALSE;
  }

  // Reduce the frame rate if possible.
  if (capability.maxFPS > requested_capability.maxFPS) {
    capability.maxFPS = requested_capability.maxFPS;
  } else if (capability.maxFPS <= 0) {
    capability.maxFPS = 30;
  }

  // Set format to capture device
  CComPtr<IMFMediaType> media_type = nullptr;
  HRESULT hr = reader_->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      capabilityIndex, &media_type);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "GetNativeMediaType failed:"
                      << std::system_category().message(hr);
    return hr;
  }

  hr = reader_->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr,
                                    media_type);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "SetCurrentMediaType failed:"
                      << std::system_category().message(hr);
    return hr;
  }

  // Store the capability that is set to capture device
  output_capability_ = capability;
  RTC_LOG(LS_INFO) << "BestCapability Selected: width:"
                   << capability.width << ", height:" << capability.height
                   << ", FPS:" << capability.maxFPS
                   << ", videoType:" << capability.videoType;

  UINT32 width, height;
  MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &width, &height);
  UINT32 numerator, denominator;
  MFGetAttributeRatio(media_type, MF_MT_FRAME_RATE, &numerator, &denominator);
  GUID subtype;
  // Get the video subType for this media type
  media_type->GetGUID(MF_MT_SUBTYPE, &subtype);
  if (subtype == MFVideoFormat_I420) {
    format = "I420";
  } else if (subtype == MFVideoFormat_IYUV) {
    format = "IYUV";
  } else if (subtype == MFVideoFormat_RGB24) {
    format = "RGB24";
  } else if (subtype == MFVideoFormat_YUY2) {
    format = "YUY2";
  } else if (subtype == MFVideoFormat_NV12) {
    format = "NV12";
  } else if (subtype == MFVideoFormat_RGB565) {
    format = "RGB565";
  } else if (subtype == MFVideoFormat_MJPG) {
    format = "MJPG";
  } else {
    format = "Others";
  }
  RTC_LOG(LS_INFO) << "VideoCapturer: Format set to camera, Width:" << width
                   << ", height:" << height
                   << ", FPS:" << numerator / denominator
                   << ", subtype:" << format;
  return hr;
}
} // namespace videocapturemodule
} // namespace webrtc
