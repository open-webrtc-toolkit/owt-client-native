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
      : objc_generator_(generator) {}

  bool GenerateFramesForNext10Ms(int8_t** frame_buffer);
  virtual int GetSampleRate();
  virtual int GetChannelNumber();

 private:
  id<RTCAudioFrameGeneratorProtocol> objc_generator_;
};

/// This class cast objc video frame generator to C++ one. Only I420 raw frame
/// is supported.
class VideoFrameGeneratorObjcImpl : public FrameGeneratorInterface {
 public:
  explicit VideoFrameGeneratorObjcImpl(
      id<RTCVideoFrameGeneratorProtocol> generator)
      : objc_generator_(generator), previous_(nullptr) {}
  virtual int GetFrameSize();
  virtual void GenerateNextFrame(uint8** frame_buffer);
  virtual int GetHeight();
  virtual int GetWidth();
  virtual int GetFps();
  virtual VideoFrameCodec GetType();

 private:
  id<RTCVideoFrameGeneratorProtocol> objc_generator_;
  uint8* previous_;
};
}
}

#endif  // WOOGEEN_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
