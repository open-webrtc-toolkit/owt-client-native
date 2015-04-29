/*
 * Intel License
 */

#include "peerconnectionhandler.h"
#include "webrtc/base/bind.h"

namespace woogeen {

  enum {
    MSG_CREATEOFFER = 1,
    MSG_CREATEANSWER,
    MSG_CREATESESSIONDESCRIPTION_SUCCESS,
    MSG_CREATESESSIONDESCRIPTION_FAILURE,
  };

  struct CreateSessionDescriptionMessageData : public rtc::MessageData {
    CreateSessionDescriptionMessageData (CreateSessionDescriptionRequest* in_request, const webrtc::MediaConstraintsInterface* in_constrants)
        : request(in_request),
          constrants(in_constrants) {}
    CreateSessionDescriptionRequest* request;
    const webrtc::MediaConstraintsInterface* constrants;
  };

  void CreateSessionDescriptionRequest::OnSuccess(SessionDescriptionInterface* desc) {
    CHECK(callback_thread_);
    rtc::TypedMessageData<SessionDescriptionInterface*>* message = new rtc::TypedMessageData<SessionDescriptionInterface*>(desc);
    callback_thread_->Send(this, MSG_CREATESESSIONDESCRIPTION_SUCCESS, message);
  }

  void CreateSessionDescriptionRequest::OnFailure(const std::string& error) {
    CHECK(callback_thread_);
    rtc::TypedMessageData<std::string>* message = new rtc::TypedMessageData<std::string>(error);
    callback_thread_->Send(this, MSG_CREATESESSIONDESCRIPTION_FAILURE, message);
  }

  void CreateSessionDescriptionRequest::OnMessage(Message* msg) {
    switch (msg->message_id) {
      case MSG_CREATESESSIONDESCRIPTION_SUCCESS: {
        rtc::TypedMessageData<SessionDescriptionInterface*>* data = static_cast<rtc::TypedMessageData<SessionDescriptionInterface*>*>(msg->pdata);
        OnRequestSuccess(data->data());
        break;
      }
      case MSG_CREATESESSIONDESCRIPTION_FAILURE: {
        rtc::TypedMessageData<std::string>* data = static_cast<rtc::TypedMessageData<std::string>*>(msg->pdata);
        OnRequestFailure(data->data());
        break;
      }
    }
  }

  PeerConnectionHandler::PeerConnectionHandler(PeerConnectionInterface* peer_connection, Thread* pc_thread, Thread* callback_thread)
    : peer_connection_(peer_connection),
      pc_thread_(pc_thread),
      callback_thread_(callback_thread) {
    CHECK(peer_connection_);
    CHECK(pc_thread_);
    CHECK(callback_thread_);
  }

  void PeerConnectionHandler::OnMessage(Message* msg) {
    switch (msg->message_id) {
      case MSG_CREATEOFFER: {
        CreateSessionDescriptionMessageData* data = static_cast<CreateSessionDescriptionMessageData*>(msg->pdata);
        peer_connection_->CreateOffer(data->request, data->constrants);
      }
    }
  }

  void PeerConnectionHandler::CreateOffer(CreateSessionDescriptionRequest* request, const webrtc::MediaConstraintsInterface* constrants) {
    CHECK(pc_thread_);
    CHECK(request);
    CHECK(constrants);
    CreateSessionDescriptionMessageData* data = new CreateSessionDescriptionMessageData(request, constrants);
    pc_thread_->Send(this, MSG_CREATEOFFER, data);
  }
}
