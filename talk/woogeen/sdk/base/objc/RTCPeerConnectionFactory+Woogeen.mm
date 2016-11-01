//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionFactory+Woogeen.h"

#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"

@implementation RTCPeerConnectionFactory (Woogeen)

+ (RTCPeerConnectionFactory*)sharedInstance {
  static RTCPeerConnectionFactory* factory;
  static dispatch_once_t token;
  dispatch_once(&token, ^{
    factory = [[RTCPeerConnectionFactory alloc]
        initWithNativePeerConnectionFactory:
            woogeen::base::PeerConnectionDependencyFactory::Get()
                ->PeerConnectionFactory()];
  });
  return factory;
}

@end
