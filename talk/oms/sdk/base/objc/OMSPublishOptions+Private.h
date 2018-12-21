// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <memory>
#import "talk/oms/sdk/include/objc/OMS/OMSPublishOptions.h"
@interface OMSPublishOptions ()
@property(nonatomic, readonly) std::shared_ptr<oms::base::PublishOptions>
    nativePublishOptions;
@end
