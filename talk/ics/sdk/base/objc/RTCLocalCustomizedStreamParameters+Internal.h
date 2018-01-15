/*
 * Copyright (c) 2016 Intel Corporation. All rights reserved.
 */

#import <memory>
#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCLocalCustomizedStreamParameters.h"

@interface RTCLocalCustomizedStreamParameters (Internal)

- (std::shared_ptr<ics::base::LocalCustomizedStreamParameters>)
    nativeParameters;
- (void)setNativeParameters:
    (std::shared_ptr<ics::base::LocalCustomizedStreamParameters>)
        nativeParameters;
- (id<RTCVideoFrameGeneratorProtocol>)videoFrameGenerator;

@end
