// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_SELFIESEGMENTATIONMODEL_H_
#define OWT_IC_SELFIESEGMENTATIONMODEL_H_

#include <string>
#include <vector>

#include "inference_engine.hpp"
#include "opencv2/core/mat.hpp"

namespace owt {
namespace ic {

class SelfieSegmentationModel {
 public:
  explicit SelfieSegmentationModel(InferenceEngine::Core& core);

  bool LoadModel(const std::string& xmlPath, const std::string& device = "CPU");
  bool IsLoaded() const;

  cv::Mat Predict(const cv::Mat& frame);
  void PredictAsync(const cv::Mat& frame);
  cv::Mat WaitForFinished();

 private:
  InferenceEngine::Core &core_;
  InferenceEngine::InferRequest request_;
  std::string input_name_;
  std::string output_name_;
  InferenceEngine::SizeVector output_size_;
};

}  // namespace ic
}  // namespace owt

#endif  // OWT_IC_SELFIESEGMENTATIONMODEL_H_
