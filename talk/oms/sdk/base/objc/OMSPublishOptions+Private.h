//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//
#include <memory>
#import "talk/oms/sdk/include/objc/OMS/OMSPublishOptions.h"
@interface OMSPublishOptions ()
@property(nonatomic, readonly) std::shared_ptr<oms::base::PublishOptions>
    nativePublishOptions;
@end
