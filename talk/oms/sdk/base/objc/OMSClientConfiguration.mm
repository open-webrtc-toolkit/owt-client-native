// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/oms/sdk/base/objc/OMSClientConfiguration+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"
@implementation OMSClientConfiguration
- (instancetype)init {
  if (self = [super init]) {
    _rtcConfiguration = [[RTCConfiguration alloc] init];
  }
  return self;
}
- (std::shared_ptr<oms::base::ClientConfiguration>)nativeClientConfiguration {
  std::shared_ptr<oms::base::ClientConfiguration> config(
      new oms::base::ClientConfiguration());
  std::vector<oms::base::IceServer> iceServers;
  for (RTCIceServer* server in _rtcConfiguration.iceServers) {
    oms::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  config->ice_servers = iceServers;
  config->candidate_network_policy =
      (_rtcConfiguration.candidateNetworkPolicy ==
       RTCCandidateNetworkPolicyLowCost)
          ? oms::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : oms::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  return config;
}
@end
