// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CLIENTCONFIGURATION_H_
#define OWT_BASE_CLIENTCONFIGURATION_H_
#include <vector>
#include <string>
#include "owt/base/commontypes.h"
#include "owt/base/network.h"
namespace owt {
namespace base{
/// Client configurations
struct ClientConfiguration {
  enum class CandidateNetworkPolicy : int { kAll = 1, kLowCost };
  ClientConfiguration()
       : candidate_network_policy(CandidateNetworkPolicy::kAll) {};
  /// List of ICE servers
  std::vector<IceServer> ice_servers;
  /**
   @brief Candidate collection policy.
   @details If you do not want cellular network when WiFi is available, please
   use CandidateNetworkPolicy::kLowCost. Using low cost policy may not have good
   network experience. Default policy is collecting all candidates.
   */
  CandidateNetworkPolicy candidate_network_policy;
};
}
}
#endif  // OWT_BASE_CLIENTCONFIGURATION_H_
