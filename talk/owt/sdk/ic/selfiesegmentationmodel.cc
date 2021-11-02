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
    : core(core) {}

bool SelfieSegmentationModel::LoadModel(const std::string& xmlPath,
                                        const std::string& device) {
  InferenceEngine::CNNNetwork network = core.ReadNetwork(xmlPath);
  if (network.getInputsInfo().empty() || network.getOutputsInfo().empty()) {
    RTC_LOG(LS_WARNING) << "Bad segmentation model: no input / output";
    return false;
  }
  inputName = network.getInputsInfo().begin()->first;
  outputName = network.getOutputsInfo().begin()->first;
  IE::DataPtr outputData = network.getOutputsInfo().begin()->second;
  outputData->setPrecision(IE::Precision::U8);
  outputSize = outputData->getTensorDesc().getDims();

  IE::PreProcessInfo& preprocess =
      network.getInputsInfo()[inputName]->getPreProcess();
  preprocess.setResizeAlgorithm(IE::ResizeAlgorithm::RESIZE_BILINEAR);

  InferenceEngine::ExecutableNetwork executableNetwork =
      core.LoadNetwork(network, device);
  request = executableNetwork.CreateInferRequest();
}

bool SelfieSegmentationModel::IsLoaded() const {
  return static_cast<bool>(request);
}

cv::Mat SelfieSegmentationModel::predict(const cv::Mat& frame) {
  predictAsync(frame);
  return waitForFinished();
}

void SelfieSegmentationModel::predictAsync(const cv::Mat& frame) {
  CV_DbgAssert(frame.type() == CV_32FC3);
  size_t height = frame.rows;
  size_t width = frame.cols;
  auto blob = IE::make_shared_blob(
      IE::TensorDesc(IE::Precision::FP32, {1, 3, height, width},
                     IE::Layout::NHWC),
      (float*)frame.data);
  request.SetBlob(inputName, blob);
  request.StartAsync();
}

cv::Mat SelfieSegmentationModel::waitForFinished() {
  request.Wait(IE::InferRequest::WaitMode::RESULT_READY);
  const unsigned char* data = request.GetBlob(outputName)->buffer();
  int height = outputSize[1];
  int width = outputSize[2];
  return cv::Mat(height, width, CV_8U, (void*)data);
}

}  // namespace ic
}  // namespace owt
