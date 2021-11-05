// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "backgroundblur.h"

#include <exception>
#include <limits>

#include "libyuv/convert.h"
#include "opencv2/imgproc.hpp"
#include "third_party/webrtc/api/video/i420_buffer.h"
#include "third_party/webrtc/rtc_base/logging.h"

#include "selfiesegmentationmodel.h"

namespace owt {
namespace ic {

BackgroundBlur::BackgroundBlur(InferenceEngine::Core& core) : model_(core) {}

bool BackgroundBlur::SetParameter(const std::string& key,
                                  const std::string& value) {
  if (key == "model_path") {
    return model_.LoadModel(value);
  } else if (key == "blur_radius") {
    blur_radius_ = stoi(value);
    return true;
  }
  return false;
}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> BackgroundBlur::Process(
    const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) {
  if (!model_.IsLoaded()) {
    RTC_LOG(LS_WARNING) << "Background blur model is not initialized.";
    return buffer;
  }

  if (buffer->type() == webrtc::VideoFrameBuffer::Type::kNative) {
    RTC_LOG(LS_WARNING) << "Native video frame buffer is not supported.";
    return buffer;
  }

  auto i420 = buffer->GetI420();
  std::vector<unsigned char> rgb(3ll * i420->width() * i420->height());
  libyuv::I420ToRAW(i420->DataY(), i420->StrideY(), i420->DataU(),
                    i420->StrideU(), i420->DataV(), i420->StrideV(), rgb.data(),
                    i420->width() * 3, i420->width(), i420->height());
  cv::Mat frame(buffer->height(), buffer->width(), CV_8UC3, rgb.data());

  cv::Mat input;
  frame.convertTo(input, CV_32FC3, 1. / UCHAR_MAX);
  cv::Mat mask = model_.Predict(input);  // mask is of 8UC1

  cv::resize(mask, mask, {buffer->width(), buffer->height()});
  cv::Mat foreground_mask;
  cv::merge(std::vector<cv::Mat>{mask, mask, mask}, foreground_mask);
  cv::Mat background_mask;
  cv::merge(std::vector<cv::Mat>{UCHAR_MAX - mask, UCHAR_MAX - mask,
                                 UCHAR_MAX - mask},
            background_mask);

  // Apply a masked blur on background
  cv::Mat masked_frame;
  cv::multiply(frame, background_mask, masked_frame, 1. / UCHAR_MAX);
  cv::Mat background;
  cv::GaussianBlur(masked_frame, background, {blur_radius_, blur_radius_}, 0);
  cv::Mat blurred_mask;
  cv::GaussianBlur(background_mask, blurred_mask, {blur_radius_, blur_radius_},
                   0);
  cv::divide(background, blurred_mask, background, UCHAR_MAX);
  cv::multiply(background, background_mask, background, 1. / UCHAR_MAX);

  cv::Mat foreground;
  cv::multiply(frame, foreground_mask, foreground, 1. / UCHAR_MAX);
  frame = foreground + background;

  auto new_buffer =
      webrtc::I420Buffer::Create(buffer->width(), buffer->height());
  libyuv::RAWToI420(frame.data, frame.cols * 3, new_buffer->MutableDataY(),
                    new_buffer->StrideY(), new_buffer->MutableDataU(),
                    new_buffer->StrideU(), new_buffer->MutableDataV(),
                    new_buffer->StrideV(), buffer->width(), buffer->height());

  return new_buffer;
}

}  // namespace ic
}  // namespace owt
