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

  virtual std::vector<uint8_t> GenerateFramesForNext10Ms();
  virtual int GetSampleRate();
  virtual int GetChannelNumber();

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
      : objc_generator_(generator) {}
  virtual std::vector<uint8_t> GenerateNextFrame();
  virtual int GetHeight();
  virtual int GetWidth();
  virtual int GetFps();
  virtual VideoFrameCodec GetType();

 private:
  id<RTCVideoFrameGeneratorProtocol> objc_generator_;
  int buffer_size_for_a_frame_;
};
}
}

#endif  // WOOGEEN_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
