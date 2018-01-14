/*
 * Intel License
 */

#include <string>
#include <functional>
#include <unordered_map>
#include "talk/woogeen/sdk/include/objc/Woogeen/RTCP2PPeerConnectionChannelObserver.h"
#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCRemoteStream.h"

namespace woogeen {
namespace p2p {
// It wraps an id<RTCP2PPeerConnectionChannelObserver> and call methods on that
// interface.
class P2PPeerConnectionChannelObserverObjcImpl
    : public P2PPeerConnectionChannelObserver {
 public:
  P2PPeerConnectionChannelObserverObjcImpl(
      id<RTCP2PPeerConnectionChannelObserver> observer);

 protected:
  void OnInvited(const std::string& remote_id) override;
  void OnAccepted(const std::string& remote_id) override;
  void OnDenied(const std::string& remote_id) override;
  void OnStarted(const std::string& remote_id) override;
  void OnStopped(const std::string& remote_id) override;
  void OnData(const std::string& remote_id,
              const std::string& message) override;
  void OnStreamAdded(
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream) override;
  void OnStreamAdded(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream) override;
  void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream) override;
  void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream) override;

 private:
  void TriggerStreamRemoved(std::shared_ptr <
                            woogeen::base::RemoteStream> stream);
  id<RTCP2PPeerConnectionChannelObserver> _observer;
  // Key is stream ID
  std::unordered_map<std::string, RTCRemoteStream*> remote_streams_;
};
}
}
