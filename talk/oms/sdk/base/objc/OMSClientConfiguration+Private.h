// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <memory>
#include "talk/oms/sdk/include/cpp/oms/base/clientconfiguration.h"
#import "talk/oms/sdk/include/objc/OMS/OMSClientConfiguration.h"
@interface OMSClientConfiguration()
@property(nonatomic, readonly) std::shared_ptr<oms::base::ClientConfiguration>
    nativeClientConfiguration;
@end
