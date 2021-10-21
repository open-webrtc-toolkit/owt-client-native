// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "backgroundblur.h"

#include "api/video/i420_buffer.h"
#include "libyuv/convert.h"
#include "opencv2/imgproc.hpp"

#include "selfiesegmentation.h"

namespace owt {
namespace ic {

BackgroundBlur::BackgroundBlur(int blurRadius)
    : model(new SelfieSegmentation(
          "C:/Users/wangzhib/Desktop/segmentation/contrib/"
          "owt-selfie-segmentation-144x256.xml")),
      blur_radius_(blurRadius) {
  if (this->blur_radius_ < 0) {
    this->blur_radius_ = 1;
  } else if (this->blur_radius_ % 2 == 0) {
    ++this->blur_radius_;
  }
}

BackgroundBlur::~BackgroundBlur() {}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> BackgroundBlur::Process(
    const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) {
  auto i420 = buffer->GetI420();
  std::vector<unsigned char> rgb(3ll * i420->width() * i420->height());
  libyuv::I420ToRAW(i420->DataY(), i420->StrideY(), i420->DataU(),
                    i420->StrideU(), i420->DataV(), i420->StrideV(), rgb.data(),
                    i420->width() * 3, i420->width(), i420->height());
  cv::Mat frame(buffer->height(), buffer->width(), CV_8UC3, rgb.data());

  cv::Mat input;
  frame.convertTo(input, CV_32FC3, 1. / 255);
  cv::Mat mask = model->predict(input);  // mask is of 8UC1

  cv::Mat resizedMask;
  cv::resize(mask, resizedMask, {buffer->width(), buffer->height()});
  cv::Mat mask3;
  std::vector<cv::Mat> masks{255 - resizedMask, 255 - resizedMask,
                             255 - resizedMask};
  cv::merge(masks.data(), 3, mask3);

  cv::Mat background;
  cv::multiply(frame, mask3, background, 1. / 255);
  frame -= background;
  cv::GaussianBlur(background, background, {blur_radius_, blur_radius_}, 0);
  cv::add(frame, background, frame);

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
