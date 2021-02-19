// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/base/objc/OWTStream+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTLocalStream.h"
#import "RTCVideoCapturer.h"
@interface OWTLocalStream ()
/// If capturer is nil, caller manages its capturer's lifetime.
@property(nonatomic, strong) RTCVideoCapturer* capturer;
- (std::shared_ptr<owt::base::LocalStream>)nativeLocalStream;
- (NSString*)streamId;
@end
