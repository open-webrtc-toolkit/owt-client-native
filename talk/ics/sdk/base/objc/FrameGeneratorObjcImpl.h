//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#ifndef ICS_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
#define ICS_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_

#include "talk/ics/sdk/include/cpp/ics/base/framegeneratorinterface.h"
#include "talk/ics/sdk/include/objc/ICS/ICSFrameGeneratorProtocol.h"

namespace ics {
namespace base {
class AudioFrameGeneratorObjcImpl : public AudioFrameGeneratorInterface {
 public:
  explicit AudioFrameGeneratorObjcImpl(
      id<RTCAudioFrameGeneratorProtocol> generator)
      : objc_generator_(generator) {
      }

  virtual uint32_t GenerateFramesForNext10Ms(uint8_t* buffer,
                                             const uint32_t capacity) override;
  virtual int GetSampleRate() override;
  virtual int GetChannelNumber() override;

 private:
  id<RTCAudioFrameGeneratorProtocol> objc_generator_;
};

/// This class cast objc video frame generator to C++ one. Only I420 raw frame
/// is supported.
class VideoFrameGeneratorObjcImpl : public VideoFrameGeneratorInterface {
 public:
  explicit VideoFrameGeneratorObjcImpl(
      id<RTCVideoFrameGeneratorProtocol> generator)
      : objc_generator_(generator), buffer_size_for_a_frame_(0) {}
  virtual uint32_t GenerateNextFrame(uint8_t* buffer,
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

#endif  // ICS_BASE_OBJC_VIDEOFRAMEGENERATOROBJCIMPL_H_
