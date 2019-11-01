// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/RTCPeerConnectionFactory+OWT.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCPeerConnectionFactory+Native.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCPeerConnectionFactory+Private.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"

@implementation RTCPeerConnectionFactory (OWT)

+ (RTCPeerConnectionFactory*)sharedInstance {
  static RTCPeerConnectionFactory* factory;
  static dispatch_once_t token;
  dispatch_once(&token, ^{
    factory = [[RTCPeerConnectionFactory alloc]
        initWithNativePeerConnectionFactory:
            owt::base::PeerConnectionDependencyFactory::Get()
                ->PeerConnectionFactory()];
  });
  return factory;
}

@end
