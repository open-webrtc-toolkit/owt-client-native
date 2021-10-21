// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_SELFIESEGMENTATION_H_
#define OWT_IC_SELFIESEGMENTATION_H_

#include <string>
#include <vector>

#include "inference_engine.hpp"
#include "opencv2/core/mat.hpp"

namespace owt {
namespace ic {

class SelfieSegmentation {
 public:
  explicit SelfieSegmentation(const std::string& xmlPath,
                              const std::string& device = "CPU");

  cv::Mat predict(const cv::Mat& frame);
  void predictAsync(const cv::Mat& frame);
  cv::Mat waitForFinished();

 private:
  InferenceEngine::Core core;
  InferenceEngine::CNNNetwork network;
  InferenceEngine::ExecutableNetwork executableNetwork;
  InferenceEngine::InferRequest request;
  std::string inputName;
  std::string outputName;
  InferenceEngine::SizeVector outputSize;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_SELFIE_SEGMENTATION_H_
