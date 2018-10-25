/*
 * Intel License
 */

#ifndef OMS_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
#define OMS_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_

#include <unordered_map>
#include <mutex>
#include "talk/oms/sdk/include/cpp/oms/conference/conferenceclient.h"

#import "talk/oms/sdk/include/objc/OMS/OMSConferenceClient.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteStream.h"

namespace oms {
namespace conference {

class ConferenceClientObserverObjcImpl : public ConferenceClientObserver {
 public:
  ConferenceClientObserverObjcImpl(OMSConferenceClient* conferenceClient,
                                   id<OMSConferenceClientDelegate> delegate);
  virtual ~ConferenceClientObserverObjcImpl(){};
  id<OMSConferenceClientDelegate> ObjcObserver() const { return delegate_; }

 protected:
  virtual void OnStreamAdded(
      std::shared_ptr<oms::base::RemoteStream> stream) override;
  virtual void OnStreamAdded(
      std::shared_ptr<oms::conference::RemoteMixedStream> stream) override;
  virtual void OnMessageReceived(std::string& message,
                                 std::string& sender_id,
                                 std::string& target_type) override;
  virtual void OnParticipantJoined(
      std::shared_ptr<oms::conference::Participant> user) override;
  virtual void OnServerDisconnected() override;

 private:
  void AddRemoteStreamToMap(const std::string& id, OMSRemoteStream* stream);
  void TriggerOnStreamRemoved(
      std::shared_ptr<oms::base::RemoteStream> stream);

  OMSConferenceClient* client_;
  id<OMSConferenceClientDelegate> delegate_;
  std::unordered_map<std::string, OMSRemoteStream*> remote_streams_;
  std::mutex remote_streams_mutex_;
  std::unordered_map<std::string, OMSLocalStream*> local_streams_;
  std::mutex local_streams_mutex_;
};
}
}

#endif  // OMS_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
