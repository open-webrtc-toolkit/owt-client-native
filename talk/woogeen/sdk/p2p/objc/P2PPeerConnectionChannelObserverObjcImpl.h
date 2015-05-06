/*
 * Intel License
 */

#include <string>
#include <functional>
#include "talk/woogeen/sdk/p2p//P2PPeerConnectionChannel.h"
#include "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannelObserver.h"

namespace woogeen {

// It wraps an id<RTCP2PPeerConnectionChannelObserver> and call methods on that interface.
class P2PPeerConnectionChannelObserverObjcImpl : public P2PPeerConnectionChannelObserver {

  public:
    P2PPeerConnectionChannelObserverObjcImpl(id<RTCP2PPeerConnectionChannelObserver> observer);

  protected:
    void OnInvited(std::string remote_id);
    void OnAccepted(std::string remote_id);
    void OnStopped(std::string remote_id);

  private:
    id<RTCP2PPeerConnectionChannelObserver> _observer;
};

}
