/*
 * Intel License
 */

#import "talk/ics/sdk/include/objc/ICS/ICSP2PSignalingSenderProtocol.h"

#include <string>
#include <functional>
#include "talk/ics/sdk/include/cpp/ics/p2p/p2psignalingsenderinterface.h"

namespace ics {
namespace p2p {

// It wraps an id<RTCSignalingSenderInterface> and call methods on that
// interface.
class RTCP2PSignalingSenderObjcImpl : public P2PSignalingSenderInterface {
 public:
  RTCP2PSignalingSenderObjcImpl(id<RTCP2PSignalingSenderProtocol> sender);
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> success,
                                    std::function<void(int)> failure);

 private:
  id<RTCP2PSignalingSenderProtocol> _sender;
};
}
}
