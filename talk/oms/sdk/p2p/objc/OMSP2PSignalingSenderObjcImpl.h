/*
 * Intel License
 */

#import "talk/oms/sdk/include/objc/OMS/OMSP2PSignalingSenderProtocol.h"

#include <string>
#include <functional>
#include "talk/oms/sdk/include/cpp/oms/p2p/p2psignalingsenderinterface.h"

namespace oms {
namespace p2p {

// It wraps an id<RTCSignalingSenderInterface> and call methods on that
// interface.
class OMSP2PSignalingSenderObjcImpl : public P2PSignalingSenderInterface {
 public:
  OMSP2PSignalingSenderObjcImpl(id<OMSP2PSignalingSenderProtocol> sender);
  virtual void SendSignalingMessage(const std::string& message,
                                    const std::string& remote_id,
                                    std::function<void()> success,
                                    std::function<void(std::unique_ptr<oms::base::Exception>)> failure);

 private:
  id<OMSP2PSignalingSenderProtocol> _sender;
};
}
}
