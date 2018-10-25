/*
 * Intel License
 */

#ifndef OMS_P2P_P2PSIGNALINGSENDERIMPL_H_
#define OMS_P2P_P2PSIGNALINGSENDERIMPL_H_

#include "talk/oms/sdk/include/cpp/oms/p2p/p2psignalingsenderinterface.h"

namespace oms {
namespace p2p {

class P2PSignalingSenderImpl
    : public P2PSignalingSenderInterface {
 public:
  explicit P2PSignalingSenderImpl(P2PSignalingSenderInterface* sender);
  virtual ~P2PSignalingSenderImpl();

  void SendSignalingMessage(const std::string& message,
                            const std::string& remote_id,
                            std::function<void()> success,
                            std::function<void(std::unique_ptr<oms::base::Exception>)> failure) override;
 private:
  P2PSignalingSenderInterface* sender_;
};
}
}

#endif  // OMS_P2P_P2PSIGNALINGSENDERIMPL_H_
