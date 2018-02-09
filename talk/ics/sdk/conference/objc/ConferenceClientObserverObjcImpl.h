/*
 * Intel License
 */

#ifndef ICS_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
#define ICS_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_

#include <unordered_map>
#include <mutex>
#include "talk/ics/sdk/include/cpp/ics/conference/conferenceclient.h"

#import "talk/ics/sdk/include/objc/ICS/ICSConferenceClient.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"

namespace ics {
namespace conference {

class ConferenceClientObserverObjcImpl : public ConferenceClientObserver {
 public:
  ConferenceClientObserverObjcImpl(ICSConferenceClient* conferenceClient,
                                   id<ICSConferenceClientDelegate> delegate);
  virtual ~ConferenceClientObserverObjcImpl(){};
  id<ICSConferenceClientDelegate> ObjcObserver() const { return delegate_; }

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
  void AddRemoteStreamToMap(const std::string& id, ICSRemoteStream* stream);
  void TriggerOnStreamRemoved(
      std::shared_ptr<ics::base::RemoteStream> stream);

  ICSConferenceClient* client_;
  id<ICSConferenceClientDelegate> delegate_;
  std::unordered_map<std::string, ICSRemoteStream*> remote_streams_;
  std::mutex remote_streams_mutex_;
  std::unordered_map<std::string, ICSLocalStream*> local_streams_;
  std::mutex local_streams_mutex_;
};
}
}

#endif  // ICS_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
