// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
#define OWT_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
#include "talk/owt/sdk/include/cpp/owt/base/framegeneratorinterface.h"
#include "talk/owt/sdk/include/objc/OWT/OWTFrameGeneratorProtocol.h"
namespace owt {
namespace base {
class AudioFrameGeneratorObjcImpl : public AudioFrameGeneratorInterface {
 public:
  explicit AudioFrameGeneratorObjcImpl(
      id<RTCAudioFrameGeneratorProtocol> generator)
      : objc_generator_(generator) {}
  uint32_t GenerateFramesForNext10Ms(uint8_t* buffer,
                                     const uint32_t capacity) override;
  int GetSampleRate() override;
  int GetChannelNumber() override;

 private:
  __weak id<RTCAudioFrameGeneratorProtocol> objc_generator_;
};
/// This class cast objc video frame generator to C++ one. Only I420 raw frame
/// is supported.
class VideoFrameGeneratorObjcImpl : public VideoFrameGeneratorInterface {
 public:
  explicit VideoFrameGeneratorObjcImpl(
      id<RTCVideoFrameGeneratorProtocol> generator)
      : objc_generator_(generator), buffer_size_for_a_frame_(0) {}
  uint32_t GenerateNextFrame(uint8_t* buffer, const uint32_t capacity) override;
  uint32_t GetNextFrameSize() override;
  int GetHeight() override;
  int GetWidth() override;
  int GetFps() override;
  VideoFrameCodec GetType() override;

 private:
  id<RTCVideoFrameGeneratorProtocol> objc_generator_;
  int buffer_size_for_a_frame_;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
