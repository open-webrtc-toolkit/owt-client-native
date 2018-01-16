//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/RTCPeerConnectionFactory+ICS.h"

#include "talk/ics/sdk/base/peerconnectiondependencyfactory.h"

@implementation RTCPeerConnectionFactory (Woogeen)

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
