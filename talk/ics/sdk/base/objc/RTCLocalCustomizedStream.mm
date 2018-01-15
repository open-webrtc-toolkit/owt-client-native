//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "talk/ics/sdk/include/objc/Woogeen/RTCLocalCustomizedStream.h"
#import "talk/ics/sdk/base/objc/RTCLocalCustomizedStreamParameters+Internal.h"
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"
#import "talk/ics/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/ics/sdk/base/objc/FrameGeneratorObjcImpl.h"

#include "talk/ics/sdk/include/cpp/ics/base/stream.h"

@implementation RTCLocalCustomizedStream

- (instancetype)initWithParameters:
    (RTCLocalCustomizedStreamParameters*)parameters {
  self = [super init];
  ics::base::LocalCustomizedStreamParameters local_parameters =
      *[parameters nativeParameters].get();
  std::unique_ptr<ics::base::VideoFrameGeneratorInterface> frame_generator(
      new ics::base::VideoFrameGeneratorObjcImpl(
          [parameters videoFrameGenerator]));
  std::shared_ptr<ics::base::LocalCustomizedStream> local_stream =
      std::make_shared<ics::base::LocalCustomizedStream>(
          std::make_shared<ics::base::LocalCustomizedStreamParameters>(
              local_parameters),
          std::move(frame_generator));
  [super setNativeStream:local_stream];
  return self;
}

@end
