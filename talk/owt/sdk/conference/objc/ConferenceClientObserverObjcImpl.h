// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
#define OWT_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
#include <unordered_map>
#include <mutex>
#include "talk/owt/sdk/include/cpp/owt/conference/conferenceclient.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceClient.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteStream.h"
namespace owt {
namespace conference {
class ConferenceClientObserverObjcImpl : public ConferenceClientObserver {
 public:
  ConferenceClientObserverObjcImpl(OWTConferenceClient* conferenceClient,
                                   id<OWTConferenceClientDelegate> delegate);
  virtual ~ConferenceClientObserverObjcImpl(){}
  id<OWTConferenceClientDelegate> ObjcObserver() const { return delegate_; }
 protected:
  virtual void OnStreamAdded(
      std::shared_ptr<owt::base::RemoteStream> stream) override;
  virtual void OnStreamAdded(
      std::shared_ptr<owt::conference::RemoteMixedStream> stream) override;
  virtual void OnMessageReceived(std::string& message,
                                 std::string& sender_id,
                                 std::string& target_type) override;
  virtual void OnParticipantJoined(
      std::shared_ptr<owt::conference::Participant> user) override;
  virtual void OnServerDisconnected() override;
 private:
  void AddRemoteStreamToMap(const std::string& id, OWTRemoteStream* stream);
  void TriggerOnStreamRemoved(
      std::shared_ptr<owt::base::RemoteStream> stream);
  __weak OWTConferenceClient* client_;
  __weak id<OWTConferenceClientDelegate> delegate_;
  std::unordered_map<std::string, OWTRemoteStream*> remote_streams_;
  std::mutex remote_streams_mutex_;
  std::unordered_map<std::string, OWTLocalStream*> local_streams_;
  std::mutex local_streams_mutex_;
};
}
}
#endif  // OWT_CONFERENCE_OBJC_CONFERENCECLIENTOBSERVEROBJCIMPL_H_
