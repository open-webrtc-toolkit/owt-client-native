//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "talk/ics/sdk/include/objc/ICS/ICSLocalStream.h"
#import <WebRTC/RTCVideoCapturer.h>

@interface ICSLocalStream ()

/// If capturer is nil, caller manages its capturer's lifetime.
@property(nonatomic, strong) RTCVideoCapturer* capturer;

- (std::shared_ptr<ics::base::LocalStream>)nativeLocalStream;
- (NSString*)streamId;

@end
