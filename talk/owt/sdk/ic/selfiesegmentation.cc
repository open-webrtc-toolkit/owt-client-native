// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "selfiesegmentation.h"

#include <algorithm>
#include <vector>

#include "talk/owt/sdk/ic/icmanager.h"

namespace IE = InferenceEngine;

namespace owt {
namespace ic {

SelfieSegmentation::SelfieSegmentation(const std::string& xmlPath,
                                       const std::string& device)
    : network(ICManager::InferenceEngineCore().ReadNetwork(xmlPath)),
      executableNetwork(),
      request() {
  assert(!network.getInputsInfo().empty());
  assert(!network.getOutputsInfo().empty());
  inputName = network.getInputsInfo().begin()->first;
  outputName = network.getOutputsInfo().begin()->first;
  IE::DataPtr outputData = network.getOutputsInfo().begin()->second;
  outputData->setPrecision(IE::Precision::U8);
  outputSize = outputData->getTensorDesc().getDims();

  IE::PreProcessInfo& preprocess =
      network.getInputsInfo()[inputName]->getPreProcess();
  preprocess.setResizeAlgorithm(IE::ResizeAlgorithm::RESIZE_BILINEAR);

  executableNetwork =
      ICManager::InferenceEngineCore().LoadNetwork(network, device);
  request = executableNetwork.CreateInferRequest();
}

cv::Mat SelfieSegmentation::predict(const cv::Mat& frame) {
  predictAsync(frame);
  return waitForFinished();
}

void SelfieSegmentation::predictAsync(const cv::Mat& frame) {
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

cv::Mat SelfieSegmentation::waitForFinished() {
  request.Wait(IE::InferRequest::WaitMode::RESULT_READY);
  const unsigned char* data = request.GetBlob(outputName)->buffer();
  int height = outputSize[1];
  int width = outputSize[2];
  return cv::Mat(height, width, CV_8U, (void*)data);
}

}  // namespace ic
}  // namespace owt
