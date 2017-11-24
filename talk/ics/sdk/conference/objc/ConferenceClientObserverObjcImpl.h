/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
#define WOOGEEN_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_

#include <unordered_map>
#include <mutex>
#include "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"

#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceClient.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceClientObserver.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCRemoteStream.h"

namespace ics {
namespace conference {

class ConferenceClientObserverObjcImpl : public ConferenceClientObserver {
 public:
  ConferenceClientObserverObjcImpl(id<RTCConferenceClientObserver> observer,
                                   RTCConferenceClient* conferenceClient);
  virtual ~ConferenceClientObserverObjcImpl(){};
  id<RTCConferenceClientObserver> ObjcObserver() const { return observer_; }

 protected:
  virtual void OnStreamAdded(
      std::shared_ptr<ics::base::RemoteCameraStream> stream) override;
  virtual void OnStreamAdded(
      std::shared_ptr<ics::base::RemoteScreenStream> stream) override;
  virtual void OnStreamAdded(
      std::shared_ptr<ics::conference::RemoteMixedStream> stream) override;
  virtual void OnMessageReceived(std::string& sender_id,
                                 std::string& message) override;
  virtual void OnParticipantJoined(
      std::shared_ptr<ics::conference::Participant> user) override;
  virtual void OnServerDisconnected() override;

 private:
  void AddRemoteStreamToMap(const std::string& id, RTCRemoteStream* stream);
  void TriggerOnStreamRemoved(
      std::shared_ptr<ics::base::RemoteStream> stream);

  id<RTCConferenceClientObserver> observer_;
  std::unordered_map<std::string, RTCRemoteStream*> remote_streams_;
  std::mutex remote_streams_mutex_;
  std::unordered_map<std::string, RTCLocalStream*> local_streams_;
  std::mutex local_streams_mutex_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
