/*
 * Copyright (c) 2016 Intel Corporation. All rights reserved.
 */

#import <memory>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/localcamerastreamparameters.h"
#import "RTCLocalCustomizedStreamParameters.h"

@interface RTCLocalCustomizedStreamParameters (Internal)

- (std::shared_ptr<woogeen::base::LocalCustomizedStreamParameters>)
    nativeParameters;
- (void)setNativeParameters:
    (std::shared_ptr<woogeen::base::LocalCustomizedStreamParameters>)
        nativeParameters;
- (id<RTCVideoFrameGeneratorProtocol>)videoFrameGenerator;

@end
