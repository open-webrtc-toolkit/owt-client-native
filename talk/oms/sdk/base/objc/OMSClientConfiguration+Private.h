//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include <memory>
#include "talk/oms/sdk/include/cpp/oms/base/clientconfiguration.h"

#import "talk/oms/sdk/include/objc/OMS/OMSClientConfiguration.h"

@interface OMSClientConfiguration()

@property(nonatomic, readonly) std::shared_ptr<oms::base::ClientConfiguration>
    nativeClientConfiguration;

@end
