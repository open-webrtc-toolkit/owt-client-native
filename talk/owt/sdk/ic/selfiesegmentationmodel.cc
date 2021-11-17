// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "selfiesegmentationmodel.h"

namespace IE = InferenceEngine;

namespace owt {
namespace ic {

SelfieSegmentationModel::SelfieSegmentationModel(IE::Core& core)
    : core_(core) {}

void SelfieSegmentationModel::ReadModel(const std::string& modelXmlPath) {
  network_ = core_.ReadNetwork(modelXmlPath);
  auto input_info = network_.getInputsInfo();
  auto output_info = network_.getOutputsInfo();
  IE_ASSERT(input_info.size() == 1);
  IE_ASSERT(output_info.size() == 1);
  input_name_ = input_info.begin()->first;
  output_name_ = output_info.begin()->first;
  IE::DataPtr outputData = output_info.begin()->second;
  outputData->setPrecision(IE::Precision::U8);
  output_shape_ = outputData->getTensorDesc().getDims();
  IE::PreProcessInfo& preprocess = input_info[input_name_]->getPreProcess();
  preprocess.setResizeAlgorithm(IE::ResizeAlgorithm::RESIZE_BILINEAR);
}

void SelfieSegmentationModel::LoadModel(const std::string& device) {
  IE::ExecutableNetwork executableNetwork = core_.LoadNetwork(network_, device);
  request_ = executableNetwork.CreateInferRequest();
}

bool SelfieSegmentationModel::IsLoaded() const {
  return static_cast<bool>(request_);
}

cv::Mat SelfieSegmentationModel::Predict(const cv::Mat& frame) {
  IE_ASSERT(frame.type() == CV_32FC3);
  IE::SizeVector inputShape = {1, 3, static_cast<size_t>(frame.rows),
                               static_cast<size_t>(frame.cols)};
  auto blob = IE::make_shared_blob(
      IE::TensorDesc(IE::Precision::FP32, inputShape, IE::Layout::NHWC),
      (float*)frame.data);
  request_.SetBlob(input_name_, blob);
  request_.Infer();
  const unsigned char* data = request_.GetBlob(output_name_)->buffer();
  size_t output_dims = output_shape_.size();
  return cv::Mat(output_shape_[output_dims - 2], output_shape_[output_dims - 1],
                 CV_8U, (void*)data);
}

}  // namespace ic
}  // namespace owt
