//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include <memory>
#include "talk/ics/sdk/include/cpp/ics/base/clientconfiguration.h"

#import "talk/ics/sdk/include/objc/ICS/ICSClientConfiguration.h"

@interface ICSClientConfiguration()

@property(nonatomic, readonly) std::shared_ptr<ics::base::ClientConfiguration>
    nativeClientConfiguration;

@end
