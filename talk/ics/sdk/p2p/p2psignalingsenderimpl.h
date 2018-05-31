/*
 * Intel License
 */

#ifndef ICS_P2P_P2PSIGNALINGSENDERIMPL_H_
#define ICS_P2P_P2PSIGNALINGSENDERIMPL_H_

#include "talk/ics/sdk/include/cpp/ics/p2p/p2psignalingsenderinterface.h"

namespace ics {
namespace p2p {

class P2PSignalingSenderImpl
    : public P2PSignalingSenderInterface {
 public:
  explicit P2PSignalingSenderImpl(P2PSignalingSenderInterface* sender);
  virtual ~P2PSignalingSenderImpl();

  void SendSignalingMessage(const std::string& message,
                            const std::string& remote_id,
                            std::function<void()> success,
                            std::function<void(int)> failure) override;
 private:
  P2PSignalingSenderInterface* sender_;
};
}
}

#endif  // ICS_P2P_P2PSIGNALINGSENDERIMPL_H_
