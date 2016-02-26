//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#include "talk/woogeen/sdk/base/objc/FrameGeneratorObjcImpl.h"

namespace woogeen {
namespace base {

int AudioFrameGeneratorObjcImpl::GetSampleRate() {
  return (int)[objc_generator_ sampleRate];
}

int AudioFrameGeneratorObjcImpl::GetChannelNumber() {
  return (int)[objc_generator_ channelNumber];
}

bool AudioFrameGeneratorObjcImpl::GenerateFramesForNext10Ms(
    int8_t** frame_buffer) {
  int8_t* buffer;
  buffer = (int8_t*)[[objc_generator_ framesForNext10Ms] bytes];
  *frame_buffer = buffer;
  return true;
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

FrameGeneratorInterface::VideoFrameCodec
VideoFrameGeneratorObjcImpl::GetType() {
  return woogeen::base::FrameGeneratorInterface::VideoFrameCodec::I420;
}

int VideoFrameGeneratorObjcImpl::GetFrameSize() {
  int size = GetWidth() * GetHeight();
  int qsize = size / 4;
  return (size + 2 * qsize);
}

void VideoFrameGeneratorObjcImpl::GenerateNextFrame(uint8** frame_buffer) {
  uint8* buffer;
  NSData* data = [objc_generator_ nextFrame];
  buffer = (uint8*)[data bytes];
  *frame_buffer = buffer;
}
}
}
