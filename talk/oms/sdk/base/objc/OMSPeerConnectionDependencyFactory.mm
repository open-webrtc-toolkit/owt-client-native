//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaConstraints+Private.h"
#import "talk/oms/sdk/base/objc/OMSPeerConnectionDependencyFactory.h"
#import "talk/oms/sdk/base/peerconnectiondependencyfactory.h"
#import "talk/oms/sdk/include/objc/OMS/RTCPeerConnectionFactory+OMS.h"
@interface OMSPeerConnectionDependencyFactory ()
@property(nonatomic, assign)
    rtc::scoped_refptr<oms::base::PeerConnectionDependencyFactory>
        nativePeerConnectionDependencyFactory;
@end
@implementation OMSPeerConnectionDependencyFactory
static OMSPeerConnectionDependencyFactory* sharedFactory;
@synthesize nativePeerConnectionDependencyFactory =
    _nativePeerConnectionDependencyFactory;
- (id)init {
  _nativePeerConnectionDependencyFactory =
      oms::base::PeerConnectionDependencyFactory::Get();
  NSLog(@"Init RTCPCDependencyFactory");
  return self;
}
+ (id)sharedOMSPeerConnectionDependencyFactory {
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
