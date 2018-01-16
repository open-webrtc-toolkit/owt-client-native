/*
 * Copyright (c) 2016 Intel Corporation. All rights reserved.
 */

#import <memory>
#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"
#import "talk/ics/sdk/include/objc/ICS/ICSLocalCustomizedStreamParameters.h"

@interface ICSLocalCustomizedStreamParameters (Internal)

- (std::shared_ptr<ics::base::LocalCustomizedStreamParameters>)
    nativeParameters;
- (void)setNativeParameters:
    (std::shared_ptr<ics::base::LocalCustomizedStreamParameters>)
        nativeParameters;
- (id<ICSVideoFrameGeneratorProtocol>)videoFrameGenerator;

@end
