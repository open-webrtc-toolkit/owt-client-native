/*
 * Intel License
 */

#import "talk/woogeen/sdk/base/objc/RTCSignalingSenderProtocol.h"

#include <string>
#include <functional>
#include "talk/woogeen/sdk/base/signalingsenderinterface.h"

namespace woogeen {

// It wraps an id<RTCSignalingSenderInterface> and call methods on that interface.
class RTCSignalingSenderObjcImpl : public SignalingSenderInterface {

  public:
    RTCSignalingSenderObjcImpl(id<RTCSignalingSenderProtocol> sender);
    virtual void SendSignalingMessage(const std::string& message, const std::string& remote_id, std::function<void()> success, std::function<void(int)> failure);

  private:
    id<RTCSignalingSenderProtocol> _sender;
};

}
