//
//  Copyright (c) 2018 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/base/objc/ICSClientConfiguration+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCIceServer+Private.h"

@implementation ICSClientConfiguration

- (std::shared_ptr<ics::base::ClientConfiguration>)nativeClientConfiguration {
  std::shared_ptr<ics::base::ClientConfiguration> config(
      new ics::base::ClientConfiguration());
  std::vector<ics::base::IceServer> iceServers;
  for (RTCIceServer* server in _rtcConfiguration.iceServers) {
    ics::base::IceServer iceServer;
    iceServer.urls = server.nativeServer.urls;
    iceServer.username = server.nativeServer.username;
    iceServer.password = server.nativeServer.password;
    iceServers.push_back(iceServer);
  }
  config->ice_servers = iceServers;
  config->candidate_network_policy =
      (_rtcConfiguration.candidateNetworkPolicy ==
       RTCCandidateNetworkPolicyLowCost)
          ? ics::base::ClientConfiguration::CandidateNetworkPolicy::kLowCost
          : ics::base::ClientConfiguration::CandidateNetworkPolicy::kAll;
  return config;
}

@end
