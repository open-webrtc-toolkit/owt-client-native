//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <vector>
#import "talk/woogeen/sdk/base/objc/FrameGeneratorObjcImpl.h"

namespace woogeen {
namespace base {

int AudioFrameGeneratorObjcImpl::GetSampleRate() {
  return (int)[objc_generator_ sampleRate];
}

int AudioFrameGeneratorObjcImpl::GetChannelNumber() {
  return (int)[objc_generator_ channelNumber];
}

std::vector<uint8_t> AudioFrameGeneratorObjcImpl::GenerateFramesForNext10Ms() {
  if (buffer_size_for_10ms_ == 0) {
    buffer_size_for_10ms_ = (GetSampleRate() / 100) * GetChannelNumber() * 2;
  }
  uint8_t* buffer_ptr = (uint8_t*)[[objc_generator_ framesForNext10Ms] bytes];
  std::vector<uint8_t> buffer(buffer_ptr, buffer_ptr + buffer_size_for_10ms_);
  return buffer;
}

int VideoFrameGeneratorObjcImpl::GetHeight() {
  return (int)[objc_generator_ resolution].height;
}

int VideoFrameGeneratorObjcImpl::GetWidth() {
  return (int)[objc_generator_ resolution].width;
}

int VideoFrameGeneratorObjcImpl::GetFps() {
  return (int)[objc_generator_ frameRate];
}

VideoFrameGeneratorInterface::VideoFrameCodec
VideoFrameGeneratorObjcImpl::GetType() {
  return woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec::I420;
}

std::vector<uint8_t> VideoFrameGeneratorObjcImpl::GenerateNextFrame() {
  if (buffer_size_for_a_frame_ == 0) {
    int size = GetWidth() * GetHeight();
    int qsize = size / 4;
    buffer_size_for_a_frame_ = size + 2 * qsize;
  }
  uint8_t* buffer_ptr = (uint8_t*)[[objc_generator_ nextFrame] bytes];
  std::vector<uint8_t> buffer(buffer_ptr, buffer_ptr + buffer_size_for_a_frame_);
  return buffer;
}
}
}
