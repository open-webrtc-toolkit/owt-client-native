/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_
#define WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_

#include <functional>
#include <memory>
#include <string>
#include "talk/woogeen/sdk/p2p/p2pexception.h"

namespace woogeen {
class P2PSignalingChannelInterfaceObserver {
 public:
  virtual void OnMessage(const std::string& message,
                         const std::string& sender) = 0;
  virtual void OnDisconnected() = 0;
};

class P2PSignalingChannelInterface {
 public:
  virtual void AddObserver(P2PSignalingChannelInterfaceObserver* observer) = 0;
  virtual void Connect(
      const std::string& token,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
  virtual void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
  virtual void SendMessage(
      const std::string& message,
      const std::string& target_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<P2PException>)> on_failure) = 0;
};
}

#endif  // WOOGEEN_P2P_P2PSIGNALINGCHANNELINTERFACE_H_