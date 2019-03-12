// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_P2P_P2PSIGNALINGSENDERIMPL_H_
#define OWT_P2P_P2PSIGNALINGSENDERIMPL_H_
#include "talk/owt/sdk/include/cpp/owt/p2p/p2psignalingsenderinterface.h"
namespace owt {
namespace p2p {
class P2PSignalingSenderImpl
    : public P2PSignalingSenderInterface {
 public:
  explicit P2PSignalingSenderImpl(P2PSignalingSenderInterface* sender);
  virtual ~P2PSignalingSenderImpl();
  void SendSignalingMessage(const std::string& message,
                            const std::string& remote_id,
                            std::function<void()> success,
                            std::function<void(std::unique_ptr<owt::base::Exception>)> failure) override;
 private:
  P2PSignalingSenderInterface* sender_;
};
}
}
#endif  // OWT_P2P_P2PSIGNALINGSENDERIMPL_H_
