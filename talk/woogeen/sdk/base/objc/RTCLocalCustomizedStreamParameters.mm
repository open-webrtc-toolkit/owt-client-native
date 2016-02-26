/*
 * Copyright (c) 2016 Intel Corporation. All rights reserved.
 */

#include <memory>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/localcamerastreamparameters.h"

#import "talk/woogeen/sdk/base/objc/RTCLocalCustomizedStreamParameters+Internal.h"

@implementation RTCLocalCustomizedStreamParameters {
  std::shared_ptr<woogeen::base::LocalCustomizedStreamParameters>
      _nativeParameters;
  id<RTCVideoFrameGeneratorProtocol> _videoFrameGenerator;
}

- (instancetype)initWithVideoEnabled:(BOOL)videoEnabled
                        audioEnabled:(BOOL)audioEnabled {
  self = [super init];
  std::shared_ptr<woogeen::base::LocalCustomizedStreamParameters> parameters(
      new woogeen::base::LocalCustomizedStreamParameters(videoEnabled,
                                                         audioEnabled));
  _nativeParameters = parameters;
  return self;
}

- (void)setVideoFrameGenerator:
    (id<RTCVideoFrameGeneratorProtocol>)videoFrameGenerator {
  _videoFrameGenerator = videoFrameGenerator;
}

@end

@implementation RTCLocalCustomizedStreamParameters (Internal)

- (std::shared_ptr<woogeen::base::LocalCustomizedStreamParameters>)
    nativeParameters {
  return _nativeParameters;
}

- (void)setNativeParameters:
    (std::shared_ptr<woogeen::base::LocalCustomizedStreamParameters>)
        nativeParameters {
  _nativeParameters = nativeParameters;
}

- (id<RTCVideoFrameGeneratorProtocol>)videoFrameGenerator {
  return _videoFrameGenerator;
}

@end
