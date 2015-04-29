/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_SIGNALINGRECEIVERINTERFACE_H_
#define WOOGEEN_BASE_SIGNALINGRECEIVERINTERFACE_H_

namespace woogeen {

// Interface for signaling receiver.
// The receiver may be a peerconnection instance which can deal with the message received.
class SignalingReceiverInterface {
  public:
    // Received signaling message.
    virtual void onIncomingMessage(std::string message) = 0;
};
}

#endif // WOOGEEN_BASE_SIGNALINGRECEIVERINTERFACE_H_
