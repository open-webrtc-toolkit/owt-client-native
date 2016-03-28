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

uint32_t AudioFrameGeneratorObjcImpl::GenerateFramesForNext10Ms(
    const uint8_t* buffer,
    const uint32_t capacity) {
  return [objc_generator_ framesForNext10Ms:buffer capacity:capacity];
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

uint32_t VideoFrameGeneratorObjcImpl::GetNextFrameSize(){
  if (buffer_size_for_a_frame_ == 0) {
    int size = GetWidth() * GetHeight();
    int qsize = size / 4;
    buffer_size_for_a_frame_ = size + 2 * qsize;
  }
  return buffer_size_for_a_frame_;
}

uint32_t VideoFrameGeneratorObjcImpl::GenerateNextFrame(
    const uint8_t* buffer,
    const uint32_t capacity) {
  return [objc_generator_ nextFrame:buffer capacity:capacity];
}
}
}
