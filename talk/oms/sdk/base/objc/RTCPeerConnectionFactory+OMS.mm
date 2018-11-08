//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//
#import "talk/oms/sdk/include/objc/OMS/RTCPeerConnectionFactory+OMS.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCPeerConnectionFactory+Native.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCPeerConnectionFactory+Private.h"
#include "talk/oms/sdk/base/peerconnectiondependencyfactory.h"

@implementation RTCPeerConnectionFactory (OMS)
+ (RTCPeerConnectionFactory*)sharedInstance {
  static RTCPeerConnectionFactory* factory;
  static dispatch_once_t token;
  dispatch_once(&token, ^{
    factory = [[RTCPeerConnectionFactory alloc]
        initWithNativePeerConnectionFactory:
            oms::base::PeerConnectionDependencyFactory::Get()
                ->PeerConnectionFactory()];
  });
  return factory;
}
@end
