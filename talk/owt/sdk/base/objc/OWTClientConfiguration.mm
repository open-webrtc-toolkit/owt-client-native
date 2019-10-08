// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/base/objc/OWTClientConfiguration+Private.h"
#import "webrtc/sdk/objc/api/peerconnection/RTCIceServer+Private.h"
@implementation OWTClientConfiguration
- (instancetype)init {
  if (self = [super init]) {
    _rtcConfiguration = [[RTCConfiguration alloc] init];
  }
  return self;
}
- (std::shared_ptr<owt::base::ClientConfiguration>)nativeClientConfiguration {
  std::shared_ptr<owt::base::ClientConfiguration> config(
      new owt::base::ClientConfiguration());
  std::vector<owt::base::IceServer> iceServers;
  for (RTCIceServer* server in _rtcConfiguration.iceServers) {
    owt::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  config->ice_servers = iceServers;
  config->candidate_network_policy =
      (_rtcConfiguration.candidateNetworkPolicy ==
       RTCCandidateNetworkPolicyLowCost)
          ? owt::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : owt::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  return config;
}
@end
