/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
#define WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_

#include "talk/woogeen/sdk/base/peerconnectionhandler.h"
#include "talk/woogeen/sdk/base/signalingsenderinterface.h"
#include "talk/woogeen/sdk/base/signalingreceiverinterface.h"

namespace woogeen {
// An instance of P2PPeerConnectionChannel manages a session for a specified remote client.
class P2PPeerConnectionChannel : public SignalingReceiverInterface{
  public:
    explicit P2PPeerConnectionChannel(SignalingSenderInterface* sender);

  private:
    void onIncomingMessage(std::string message) override;
    SignalingSenderInterface* signaling_sender_;
};
}

#endif // WOOGEEN_P2P_P2PPEERCONNECTIONCHANNEL_H_
