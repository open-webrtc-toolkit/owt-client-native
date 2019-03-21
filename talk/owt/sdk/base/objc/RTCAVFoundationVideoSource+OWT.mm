// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "OWT/OWTAVFoundationVideoSource+Woogeen.h"
#import "talk/owt/sdk/base/objc/OWTPeerConnectionFactory+Woogeen.h"
#import "OWT/OWTAVFoundationVideoSource+Woogeen.h"
#import "webrtc/sdk/objc/Framework/Classes/avfoundationvideocapturer.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCAVFoundationVideoSource+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCMediaConstraints+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCVideoSource+Private.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
@implementation RTCAVFoundationVideoSource (Woogeen)
- (instancetype)initWithConstraints:(RTCMediaConstraints*)constraints {
  RTCPeerConnectionFactory* factory = [RTCPeerConnectionFactory sharedInstance];
  return [self initWithFactory:factory constraints:constraints];
}
- (void)setFilter:(id<RTCVideoFrameFilterProtocol>)filter {
  self.capturer->SetFilter(filter);
}
- (void)removeFilter {
  self.capturer->RemoveFilter();
}
- (void)setOutputPixelFormat:(OSType)outputPixelFormat {
  self.capturer->SetOutputPixelFormat(outputPixelFormat);
}
@end
