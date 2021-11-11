// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "selfiesegmentationmodel.h"

#include <algorithm>
#include <vector>

#include "talk/owt/sdk/ic/icmanager.h"
#include "third_party/webrtc/rtc_base/logging.h"

namespace IE = InferenceEngine;

namespace owt {
namespace ic {

SelfieSegmentationModel::SelfieSegmentationModel(InferenceEngine::Core& core)
    : core_(core) {}

void SelfieSegmentationModel::LoadModel(const std::string& xmlPath,
                                        const std::string& device) {
  InferenceEngine::CNNNetwork network = core_.ReadNetwork(xmlPath);
  IE_ASSERT(network.getInputsInfo().empty() ||
            network.getOutputsInfo().empty());
  input_name_ = network.getInputsInfo().begin()->first;
  output_name_ = network.getOutputsInfo().begin()->first;
  IE::DataPtr outputData = network.getOutputsInfo().begin()->second;
  outputData->setPrecision(IE::Precision::U8);
  output_shape_ = outputData->getTensorDesc().getDims();

  IE::PreProcessInfo& preprocess =
      network.getInputsInfo()[input_name_]->getPreProcess();
  preprocess.setResizeAlgorithm(IE::ResizeAlgorithm::RESIZE_BILINEAR);

  InferenceEngine::ExecutableNetwork executableNetwork =
      core_.LoadNetwork(network, device);
  request_ = executableNetwork.CreateInferRequest();
}

bool SelfieSegmentationModel::IsLoaded() const {
  return static_cast<bool>(request_);
}

cv::Mat SelfieSegmentationModel::Predict(const cv::Mat& frame) {
  CV_DbgAssert(frame.type() == CV_32FC3);
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
