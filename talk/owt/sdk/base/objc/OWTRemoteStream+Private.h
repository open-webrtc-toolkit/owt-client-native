// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteStream.h"
@interface OWTRemoteStream ()
- (std::shared_ptr<owt::base::RemoteStream>)nativeRemoteStream;
@end
