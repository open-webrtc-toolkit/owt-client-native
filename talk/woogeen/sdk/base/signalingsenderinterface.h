/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_SIGNALINGSENDERINTERFACE_H_
#define WOOGEEN_BASE_SIGNALINGSENDERINTERFACE_H_

#include <string>
#include <functional>

namespace woogeen {

// Interface for signaling sender.
// The sender may be a PeerClient/ConferenceClient instance which can send out signaling messages created from peerconnection.
class SignalingSenderInterface {
  public:
    // Send a signaling message.
    virtual void Send(const std::string& message, const std::string& remote_id, std::function<void()> success, std::function<void(int)> failure) = 0;
};
}

#endif // WOOGEEN_BASE_SIGNALINGSENDERINTERFACE_H_
