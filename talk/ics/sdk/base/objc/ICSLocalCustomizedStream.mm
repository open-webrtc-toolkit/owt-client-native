//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "talk/ics/sdk/include/objc/ICS/ICSLocalCustomizedStream.h"
#import "talk/ics/sdk/base/objc/ICSLocalCustomizedStreamParameters+Internal.h"
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "talk/ics/sdk/base/objc/ICSLocalStream+Internal.h"
#import "talk/ics/sdk/base/objc/FrameGeneratorObjcImpl.h"

#include "talk/ics/sdk/include/cpp/ics/base/stream.h"

@implementation ICSLocalCustomizedStream

- (instancetype)initWithParameters:
    (RTCLocalCustomizedStreamParameters*)parameters {
  self = [super init];
  ics::base::LocalCustomizedStreamParameters local_parameters =
      *[parameters nativeParameters].get();
  std::unique_ptr<ics::base::VideoFrameGeneratorInterface> frame_generator(
      new ics::base::VideoFrameGeneratorObjcImpl(
          [parameters videoFrameGenerator]));
  std::shared_ptr<ics::base::LocalStream> local_stream =
      ics::base::LocalStream::Create(
          std::make_shared<ics::base::LocalCustomizedStreamParameters>(
              local_parameters),
          std::move(frame_generator));
  [super setNativeStream:local_stream];
  return self;
}

@end
