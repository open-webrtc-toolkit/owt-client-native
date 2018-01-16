//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaConstraints+Private.h"
#import "talk/ics/sdk/base/objc/ICSPeerConnectionDependencyFactory.h"
#import "talk/ics/sdk/base/peerconnectiondependencyfactory.h"

@interface ICSPeerConnectionDependencyFactory ()

@property(nonatomic, assign)
    rtc::scoped_refptr<ics::base::PeerConnectionDependencyFactory>
        nativePeerConnectionDependencyFactory;

@end

@implementation ICSPeerConnectionDependencyFactory

static ICSPeerConnectionDependencyFactory* sharedFactory;

@synthesize nativePeerConnectionDependencyFactory =
    _nativePeerConnectionDependencyFactory;

- (id)init {
  _nativePeerConnectionDependencyFactory =
      ics::base::PeerConnectionDependencyFactory::Get();
  NSLog(@"Init RTCPCDependencyFactory");
  return self;
}

+ (id)sharedICSPeerConnectionDependencyFactory {
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
  return [[RTCMediaStream alloc] initWithNativeMediaStream:nativeMediaStream];
}

@end
