// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteStream.h"
@interface OMSRemoteStream ()
- (std::shared_ptr<oms::base::RemoteStream>)nativeRemoteStream;
@end
