// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <memory>
#include "talk/owt/sdk/include/cpp/owt/base/clientconfiguration.h"
#import "talk/owt/sdk/include/objc/OWT/OWTClientConfiguration.h"
@interface OWTClientConfiguration()
@property(nonatomic, readonly) std::shared_ptr<owt::base::ClientConfiguration>
    nativeClientConfiguration;
@end
