// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <memory>
#import "talk/owt/sdk/include/objc/OWT/OWTPublishOptions.h"
@interface OWTPublishOptions ()
@property(nonatomic, readonly) std::shared_ptr<owt::base::PublishOptions>
    nativePublishOptions;
@end
