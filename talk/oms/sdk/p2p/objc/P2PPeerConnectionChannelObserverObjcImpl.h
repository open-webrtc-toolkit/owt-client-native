/*
 * Intel License
 */

#include <string>
#include <functional>
#include <unordered_map>
#include "talk/oms/sdk/include/objc/OMS/OMSP2PPeerConnectionChannelObserver.h"
#include "talk/oms/sdk/p2p/p2ppeerconnectionchannel.h"

#import "talk/oms/sdk/include/objc/OMS/OMSRemoteStream.h"

namespace oms {
namespace p2p {
// It wraps an id<OMSP2PPeerConnectionChannelObserver> and call methods on that
// interface.
class P2PPeerConnectionChannelObserverObjcImpl
    : public P2PPeerConnectionChannelObserver {
 public:
  P2PPeerConnectionChannelObserverObjcImpl(
      id<OMSP2PPeerConnectionChannelObserver> observer);

 protected:
  void OnDenied(const std::string& remote_id) override;
  void OnStarted(const std::string& remote_id) override;
  void OnStopped(const std::string& remote_id) override;
  void OnData(const std::string& remote_id,
              const std::string& message) override;
  void OnStreamAdded(
      std::shared_ptr<oms::base::RemoteStream> stream) override;
  // Jianjun TODO: Remove OnStreamRemoved event
  void OnStreamRemoved(
      std::shared_ptr<oms::base::RemoteStream> stream);

 private:
  void TriggerStreamRemoved(std::shared_ptr <
                            oms::base::RemoteStream> stream);
  id<OMSP2PPeerConnectionChannelObserver> _observer;
  // Key is stream ID
  std::unordered_map<std::string, OMSRemoteStream*> remote_streams_;
};
}
}
