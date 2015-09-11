//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef conference_SocketSignalingChannel_h
#define conference_SocketSignalingChannel_h

#include <memory>
#include "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#include "talk/woogeen/include/sio_client.h"
#include "talk/woogeen/include/sio_message.h"

namespace woogeen {
  class SocketSignalingChannel : public ConferenceSignalingChannelInterface {
  public:
    explicit SocketSignalingChannel();
    ~SocketSignalingChannel();

    virtual void AddObserver(ConferenceSignalingChannelObserver& observer);
    virtual void RemoveObserver(ConferenceSignalingChannelObserver& observer);
    // |users| is part of |room_info|, but we can better performance if provide
    // |users| separately, because sio::message -> User is shorter than
    // sio::message -> Json::Value -> User.
    virtual void Connect(const std::string& token, std::function<void(Json::Value &room_info, std::vector<const conference::User> users)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void SendSdp(Json::Value &options, std::string &sdp, bool is_publish, std::function<void(Json::Value &ack, std::string& stream_id)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void SendStreamEvent(const std::string& event, const std::string& stream_id, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);
    virtual void SendCustomMessage(const std::string& message, const std::string& receiver, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void SendStreamControlMessage(const std::string& stream_id, const std::string& action, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void Disconnect(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);

  protected:
    virtual void OnEmitAck(sio::message::list const& msg, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  private:
    Json::Value ParseStream(const sio::message::ptr& stream);
    conference::User ParseUser(const sio::message::ptr& user_message);
    sio::client *socket_client_;
    std::vector<ConferenceSignalingChannelObserver*> observers_;
  };
}


#endif
