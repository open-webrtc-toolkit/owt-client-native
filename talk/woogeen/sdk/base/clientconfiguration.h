/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_CLIENTCONFIGURATION_H_
#define WOOGEEN_BASE_CLIENTCONFIGURATION_H_

#include <vector>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/woogeen/sdk/base/mediacodec.h"

namespace woogeen {

// Client configurations
struct ClientConfiguration {
  std::vector<webrtc::PeerConnectionInterface::IceServer> ice_servers;
  MediaCodec media_codec;
};
}

#endif  // WOOGEEN_BASE_CLIENTCONFIGURATION_H_
