// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <string>
#include <functional>
#include <unordered_map>
#include "talk/owt/sdk/include/objc/OWT/OWTP2PPeerConnectionChannelObserver.h"
#include "talk/owt/sdk/p2p/p2ppeerconnectionchannel.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteStream.h"
namespace owt {
namespace p2p {
// It wraps an id<OWTP2PPeerConnectionChannelObserver> and call methods on that
// interface.
class P2PPeerConnectionChannelObserverObjcImpl
    : public P2PPeerConnectionChannelObserver {
 public:
  P2PPeerConnectionChannelObserverObjcImpl(
      id<OWTP2PPeerConnectionChannelObserver> observer);
 protected:
  void OnMessageReceived(const std::string& remote_id,
                         const std::string& message) override;
  void OnStreamAdded(
      std::shared_ptr<owt::base::RemoteStream> stream) override;
  // Jianjun TODO: Remove OnStreamRemoved event
  void OnStreamRemoved(
      std::shared_ptr<owt::base::RemoteStream> stream);
 private:
  void TriggerStreamRemoved(std::shared_ptr <
                            owt::base::RemoteStream> stream);
  id<OWTP2PPeerConnectionChannelObserver> _observer;
  // Key is stream ID
  std::unordered_map<std::string, OWTRemoteStream*> remote_streams_;
};
}
}
