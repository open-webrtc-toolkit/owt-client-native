//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "talk/woogeen/sdk/base/objc/public/RTCLocalCustomizedStream.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalCustomizedStreamParameters+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/FrameGeneratorObjcImpl.h"

#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"

@implementation RTCLocalCustomizedStream

- (instancetype)initWithParameters:
    (RTCLocalCustomizedStreamParameters*)parameters {
  self = [super init];
  woogeen::base::LocalCustomizedStreamParameters local_parameters =
      *[parameters nativeParameters].get();
  woogeen::base::FrameGeneratorInterface* frame_generator =
      new woogeen::base::VideoFrameGeneratorObjcImpl(
          [parameters videoFrameGenerator]);
  std::shared_ptr<woogeen::base::LocalCustomizedStream> local_stream =
      std::make_shared<woogeen::base::LocalCustomizedStream>(
          std::make_shared<woogeen::base::LocalCustomizedStreamParameters>(
              local_parameters),
          frame_generator);
  [super setNativeStream:local_stream];
  return self;
}

@end
