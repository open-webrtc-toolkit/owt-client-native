#ifndef P2PSOCKETSIGNALINGCHANNEL_H
#define P2PSOCKETSIGNALINGCHANNEL_H

#include <vector>
#include "sio_client.h"
#include "woogeen/p2p/p2psignalingchannelinterface.h"

class P2PSocketSignalingChannel : public woogeen::p2p::P2PSignalingChannelInterface {
 public:
  explicit P2PSocketSignalingChannel();
  virtual void AddObserver(
      woogeen::p2p::P2PSignalingChannelInterfaceObserver& observer) override;
  virtual void RemoveObserver(
      woogeen::p2p::P2PSignalingChannelInterfaceObserver& observer) override;
  virtual void Connect(const std::string& token,
                       std::function<void()>
                           on_success,
                       std::function<void(std::unique_ptr<woogeen::p2p::P2PException>)>
                           on_failure) override;
  virtual void Disconnect(std::function<void()> on_success,
                          std::function<void(std::unique_ptr<woogeen::p2p::P2PException>)>
                              on_failure) override;
  virtual void SendMessage(const std::string& message,
                           const std::string& target_id,
                           std::function<void()>
                               on_success,
                           std::function<void(std::unique_ptr<woogeen::p2p::P2PException>)>
                               on_failure) override;

 private:
   std::vector<woogeen::p2p::P2PSignalingChannelInterfaceObserver*>
      observers_;  // Observers bind to this signaling channel instance
  std::unique_ptr<sio::client> io_;
};
#endif  // P2PSOCKETSIGNALINGCHANNEL_H
