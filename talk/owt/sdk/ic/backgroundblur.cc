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

BackgroundBlur::BackgroundBlur(InferenceEngine::Core& core) : model(core) {}

bool BackgroundBlur::SetParameter(const std::string& key,
                                  const std::string& value) {
  if (key == "model_path") {
    return model.LoadModel(value);
  } else if (key == "blur_radius") {
    blur_radius_ = stoi(value);
    return true;
  }
  return false;
}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> BackgroundBlur::Process(
    const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) {
  if (!model.IsLoaded()) {
    RTC_LOG(LS_WARNING) << "Background blur model is not initialized.";
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
  cv::Mat mask = model.predict(input);  // mask is of 8UC1

  cv::resize(mask, mask, {buffer->width(), buffer->height()});
  cv::Mat foregroundMask;
  cv::merge(std::vector<cv::Mat>{mask, mask, mask}, foregroundMask);
  cv::Mat backgroundMask;
  cv::merge(std::vector<cv::Mat>{UCHAR_MAX - mask, UCHAR_MAX - mask,
                                 UCHAR_MAX - mask},
            backgroundMask);

  // apply a masked blur on background
  cv::Mat maskedFrame;
  cv::multiply(frame, backgroundMask, maskedFrame, 1. / UCHAR_MAX);
  cv::Mat background;
  cv::GaussianBlur(maskedFrame, background, {blur_radius_, blur_radius_}, 0);
  cv::Mat blurredMask;
  cv::GaussianBlur(backgroundMask, blurredMask, {blur_radius_, blur_radius_},
                   0);
  cv::divide(background, blurredMask, background, UCHAR_MAX);
  cv::multiply(background, backgroundMask, background, 1. / UCHAR_MAX);

  cv::Mat foreground;
  cv::multiply(frame, foregroundMask, foreground, 1. / UCHAR_MAX);
  frame = foreground + background;

  auto newBuffer =
      webrtc::I420Buffer::Create(buffer->width(), buffer->height());
  libyuv::RAWToI420(frame.data, frame.cols * 3, newBuffer->MutableDataY(),
                    newBuffer->StrideY(), newBuffer->MutableDataU(),
                    newBuffer->StrideU(), newBuffer->MutableDataV(),
                    newBuffer->StrideV(), buffer->width(), buffer->height());

  return newBuffer;
}

}  // namespace ic
}  // namespace owt
