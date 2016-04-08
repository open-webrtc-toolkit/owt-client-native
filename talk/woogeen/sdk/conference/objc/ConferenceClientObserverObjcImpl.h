/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
#define WOOGEEN_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_

#include <unordered_map>
#include <mutex>
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceclient.h"

#import "talk/woogeen/sdk/conference/objc/public/RTCConferenceClientObserver.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"

namespace woogeen {
namespace conference {

class ConferenceClientObserverObjcImpl : public ConferenceClientObserver {
 public:
  ConferenceClientObserverObjcImpl(id<RTCConferenceClientObserver> observer);

 protected:
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream) override;
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream) override;
  virtual void OnStreamAdded(
      std::shared_ptr<woogeen::conference::RemoteMixedStream> stream) override;
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteCameraStream> stream) override;
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteScreenStream> stream) override;
  virtual void OnStreamRemoved(
      std::shared_ptr<woogeen::conference::RemoteMixedStream> stream) override;
  virtual void OnMessageReceived(std::string& sender_id,
                                 std::string& message) override;
  virtual void OnUserJoined(
      std::shared_ptr<const woogeen::conference::User> user) override;
  virtual void OnUserLeft(
      std::shared_ptr<const woogeen::conference::User> user) override;
  virtual void OnServerDisconnected() override;

 private:
  void AddRemoteStreamToMap(const std::string& id, RTCRemoteStream* stream);
  void TriggerOnStreamRemoved(
      std::shared_ptr<woogeen::base::RemoteStream> stream);

  id<RTCConferenceClientObserver> observer_;
  std::unordered_map<std::string, RTCRemoteStream*> remote_streams_;
  std::mutex remote_streams_mutex_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
