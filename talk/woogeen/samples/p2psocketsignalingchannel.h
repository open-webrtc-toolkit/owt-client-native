#include <vector>
#include "talk/woogeen/sdk/p2p/p2psignalingchannelinterface.h"
#include "sio_client.h"

namespace woogeensample{
  class P2PSocketSignalingChannel : public woogeen::P2PSignalingChannelInterface{
  public:
    explicit P2PSocketSignalingChannel();
    virtual void AddObserver(woogeen::P2PSignalingChannelInterfaceObserver* observer) override;
    virtual void Connect(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<woogeen::P2PException>)> on_failure) override;
    virtual void Disconnect(std::function<void()> on_success, std::function<void(std::unique_ptr<woogeen::P2PException>)> on_failure) override;
    virtual void SendMessage(const std::string& message, const std::string& target_id, std::function<void()> on_success, std::function<void(std::unique_ptr<woogeen::P2PException>)> on_failure) override;
  private:
    std::vector<woogeen::P2PSignalingChannelInterfaceObserver*> observers_;  // Observers bind to this signaling channel instance
    std::unique_ptr<sio::client> io_;
  };
}