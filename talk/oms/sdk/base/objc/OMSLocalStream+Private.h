//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//
#import "talk/oms/sdk/base/objc/OMSStream+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSLocalStream.h"
#import <WebRTC/RTCVideoCapturer.h>
@interface OMSLocalStream ()
/// If capturer is nil, caller manages its capturer's lifetime.
@property(nonatomic, strong) RTCVideoCapturer* capturer;
- (std::shared_ptr<oms::base::LocalStream>)nativeLocalStream;
- (NSString*)streamId;
@end
