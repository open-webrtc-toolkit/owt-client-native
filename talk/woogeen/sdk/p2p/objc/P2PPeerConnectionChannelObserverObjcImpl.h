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
    void OnInvited(const std::string& remote_id) override;
    void OnAccepted(const std::string& remote_id) override;
    void OnDenied(const std::string& remote_id) override;
    void OnStopped(const std::string& remote_id) override;
    void OnStreamAdded(std::shared_ptr<woogeen::RemoteCameraStream> stream) override;
    void OnStreamAdded(std::shared_ptr<woogeen::RemoteScreenStream> stream) override;
    void OnStreamRemoved(std::shared_ptr<woogeen::RemoteCameraStream> stream) override;
    void OnStreamRemoved(std::shared_ptr<woogeen::RemoteScreenStream> stream) override;

  private:
    id<RTCP2PPeerConnectionChannelObserver> _observer;
};

}
