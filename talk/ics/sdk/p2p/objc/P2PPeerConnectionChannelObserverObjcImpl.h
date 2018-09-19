/*
 * Intel License
 */

#include <string>
#include <functional>
#include <unordered_map>
#include "talk/ics/sdk/include/objc/ICS/ICSP2PPeerConnectionChannelObserver.h"
#include "talk/ics/sdk/p2p/p2ppeerconnectionchannel.h"

#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"

namespace ics {
namespace p2p {
// It wraps an id<ICSP2PPeerConnectionChannelObserver> and call methods on that
// interface.
class P2PPeerConnectionChannelObserverObjcImpl
    : public P2PPeerConnectionChannelObserver {
 public:
  P2PPeerConnectionChannelObserverObjcImpl(
      id<ICSP2PPeerConnectionChannelObserver> observer);

 protected:
  void OnDenied(const std::string& remote_id) override;
  void OnStarted(const std::string& remote_id) override;
  void OnStopped(const std::string& remote_id) override;
  void OnData(const std::string& remote_id,
              const std::string& message) override;
  void OnStreamAdded(
      std::shared_ptr<ics::base::RemoteStream> stream) override;
  // Jianjun TODO: Remove OnStreamRemoved event
  void OnStreamRemoved(
      std::shared_ptr<ics::base::RemoteStream> stream);

 private:
  void TriggerStreamRemoved(std::shared_ptr <
                            ics::base::RemoteStream> stream);
  id<ICSP2PPeerConnectionChannelObserver> _observer;
  // Key is stream ID
  std::unordered_map<std::string, ICSRemoteStream*> remote_streams_;
};
}
}
