// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "webrtc/sdk/objc/api/peerconnection/RTCMediaStream+Private.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCMediaConstraints+Private.h"
#import "talk/owt/sdk/base/objc/OWTPeerConnectionDependencyFactory.h"
#import "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#import "talk/owt/sdk/include/objc/OWT/RTCPeerConnectionFactory+OWT.h"
@interface OWTPeerConnectionDependencyFactory ()
@property(nonatomic, assign)
    rtc::scoped_refptr<owt::base::PeerConnectionDependencyFactory>
        nativePeerConnectionDependencyFactory;
@end
@implementation OWTPeerConnectionDependencyFactory
static OWTPeerConnectionDependencyFactory* sharedFactory;
@synthesize nativePeerConnectionDependencyFactory =
    _nativePeerConnectionDependencyFactory;
- (id)init {
  _nativePeerConnectionDependencyFactory =
      owt::base::PeerConnectionDependencyFactory::Get();
  NSLog(@"Init RTCPCDependencyFactory");
  return self;
}
+ (id)sharedOWTPeerConnectionDependencyFactory {
  @synchronized(self) {
    if (sharedFactory == nil) {
      sharedFactory = [[self alloc] init];
    }
  }
  return sharedFactory;
}
- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label {
  rtc::scoped_refptr<webrtc::MediaStreamInterface> nativeMediaStream =
      self.nativePeerConnectionDependencyFactory->CreateLocalMediaStream(
          [label UTF8String]);
  return [[RTCMediaStream alloc]
        initWithFactory:[RTCPeerConnectionFactory sharedInstance]
      nativeMediaStream:nativeMediaStream];
}
@end
