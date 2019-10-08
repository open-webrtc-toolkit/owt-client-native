// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTStream.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "webrtc/rtc_base/scoped_ref_ptr.h"
@interface OWTStream ()
@property(nonatomic, readwrite) std::shared_ptr<owt::base::Stream> nativeStream;
- (instancetype)initWithNativeStream:(std::shared_ptr<owt::base::Stream>)stream;
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(OWTStreamSourceInfo*)source;
- (void)setNativeStream:(std::shared_ptr<owt::base::Stream>)stream;
- (std::shared_ptr<owt::base::Stream>)nativeStream;
@end
