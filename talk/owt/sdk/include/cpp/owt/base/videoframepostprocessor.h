// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_VIDEOFRAMEPOSTPROCESSOR_H_
#define OWT_BASE_VIDEOFRAMEPOSTPROCESSOR_H_

#include <string>

#include "third_party/webrtc/rtc_base/logging.h"

namespace rtc {
template <typename>
class scoped_refptr;
}

namespace webrtc {
class VideoFrameBuffer;
}

namespace owt {
namespace base {

/// Post processor that will be applied on the video frame. This is the abstract
/// base class of the post processors provided by OWT plugins.
class VideoFramePostProcessor {
 public:
  virtual ~VideoFramePostProcessor() {}

  /**
    @brief Read the model from the specific file path.
    @param modelXmlPath The path to IR model description .xml file.
    @return Whether the process succeeds.
  */
  virtual bool ReadModel(const std::string& modelXmlPath) {
    RTC_LOG(LS_ERROR) << "ReadModel is not implemented";
    return false;
  }

  /**
    @brief Read the model to specific device.
    @param device The device name. Must be "CPU" for now.
    @return Whether the process succeeds.
  */
  virtual bool LoadModel(const std::string& device) {
    RTC_LOG(LS_ERROR) << "LoadModel is not implemented";
    return false;
  }

  /**
    @brief Set the parameter of the processor. The content of key and value
    should follow the definition of the sub-class, see documents for more
    detail.
    @return Whether the process succeeds.
  */
  virtual bool SetParameter(const std::string& key, int value) {
    RTC_LOG(LS_ERROR) << "Set parameter failed: Unknown key " << key;
    return false;
  }

  /**
    @brief Process the VideoFrameBuffer. Implemented by OWT plugins.
  */
  virtual rtc::scoped_refptr<webrtc::VideoFrameBuffer> Process(
      const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer) = 0;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_IC_VIDEOFRAMEPOSTPROCESSOR_H_
