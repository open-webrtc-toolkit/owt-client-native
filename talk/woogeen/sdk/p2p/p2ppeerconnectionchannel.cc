/*
 * Intel License
 */

#include "p2ppeerconnectionchannel.h"

namespace woogeen {
P2PPeerConnectionChannel::P2PPeerConnectionChannel(SignalingSenderInterface* sender)
    :signaling_sender_(sender) {
  CHECK(signaling_sender_);
}
}
