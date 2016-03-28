//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
#define WOOGEEN_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_

#include "talk/woogeen/sdk/include/cpp/woogeen/base/framegeneratorinterface.h"
#include "talk/woogeen/sdk/base/objc/public/RTCFrameGeneratorProtocol.h"

namespace woogeen {
namespace base {
class AudioFrameGeneratorObjcImpl : public AudioFrameGeneratorInterface {
 public:
  explicit AudioFrameGeneratorObjcImpl(
      id<RTCAudioFrameGeneratorProtocol> generator)
      : objc_generator_(generator), buffer_size_for_10ms_(0) {
      }

  virtual uint32_t GenerateFramesForNext10Ms(const uint8_t* buffer,
                                             const uint32_t capacity) override;
  virtual int GetSampleRate() override;
  virtual int GetChannelNumber() override;

 private:
  id<RTCAudioFrameGeneratorProtocol> objc_generator_;
  int buffer_size_for_10ms_;
};

/// This class cast objc video frame generator to C++ one. Only I420 raw frame
/// is supported.
class VideoFrameGeneratorObjcImpl : public VideoFrameGeneratorInterface {
 public:
  explicit VideoFrameGeneratorObjcImpl(
      id<RTCVideoFrameGeneratorProtocol> generator)
      : objc_generator_(generator), buffer_size_for_a_frame_(0) {}
  virtual uint32_t GenerateNextFrame(const uint8_t* buffer,
                                     const uint32_t capacity) override;
  virtual uint32_t GetNextFrameSize() override;
  virtual int GetHeight() override;
  virtual int GetWidth() override;
  virtual int GetFps() override;
  virtual VideoFrameCodec GetType() override;

 private:
  id<RTCVideoFrameGeneratorProtocol> objc_generator_;
  int buffer_size_for_a_frame_;
};
}
}

#endif  // WOOGEEN_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
