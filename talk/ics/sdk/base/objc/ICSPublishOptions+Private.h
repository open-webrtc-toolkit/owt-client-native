//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#include <memory>

#import "talk/ics/sdk/include/objc/ICS/ICSPublishOptions.h"

@interface ICSPublishOptions ()

@property(nonatomic, readonly) std::shared_ptr<ics::base::PublishOptions>
    nativePublishOptions;

@end
