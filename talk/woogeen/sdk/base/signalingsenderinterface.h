/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_SIGNALINGSENDERINTERFACE_H_
#define WOOGEEN_BASE_SIGNALINGSENDERINTERFACE_H_

namespace woogeen {

// Interface for signaling sender.
// The sender may be a PeerClient/ConferenceClient instance which can send out signaling messages created from peerconnection.
class SignalingSenderInterface {
  public:
    // Received signaling message.
    virtual onOutcomeMessage(std::string message) = 0;
};
}

#endif // WOOGEEN_BASE_SIGNALINGSENDERINTERFACE_H_
