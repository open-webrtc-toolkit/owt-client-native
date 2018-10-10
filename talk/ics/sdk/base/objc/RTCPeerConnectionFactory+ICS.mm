//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/RTCPeerConnectionFactory+ICS.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCPeerConnectionFactory+Native.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCPeerConnectionFactory+Private.h"

#include "talk/ics/sdk/base/peerconnectiondependencyfactory.h"


@implementation RTCPeerConnectionFactory (ICS)

+ (RTCPeerConnectionFactory*)sharedInstance {
  static RTCPeerConnectionFactory* factory;
  static dispatch_once_t token;
  dispatch_once(&token, ^{
    factory = [[RTCPeerConnectionFactory alloc]
        initWithNativePeerConnectionFactory:
            ics::base::PeerConnectionDependencyFactory::Get()
                ->PeerConnectionFactory()];
  });
  return factory;
}

@end
