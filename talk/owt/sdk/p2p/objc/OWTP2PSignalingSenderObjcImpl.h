// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTP2PSignalingSenderProtocol.h"
#include <string>
#include <functional>
#include "talk/owt/sdk/include/cpp/owt/p2p/p2psignalingsenderinterface.h"
namespace owt {
namespace p2p {
// It wraps an id<RTCSignalingSenderInterface> and call methods on that
// interface.
class OWTP2PSignalingSenderObjcImpl : public P2PSignalingSenderInterface {
 public:
  OWTP2PSignalingSenderObjcImpl(id<OWTP2PSignalingSenderProtocol> sender);
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> success,
                                    std::function<void(std::unique_ptr<owt::base::Exception>)> failure);
 private:
  id<OWTP2PSignalingSenderProtocol> _sender;
};
}
}
