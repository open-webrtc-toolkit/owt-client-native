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
    virtual void AddObserver(std::shared_ptr<ConferenceSignalingChannelObserver> observer);
    virtual void RemoveObserver(std::shared_ptr<ConferenceSignalingChannelObserver> observer);
    virtual void Connect(const std::string& token, std::function<void(Json::Value &room_info)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void SendSdp(Json::Value &options, std::string &sdp, bool is_publish, std::function<void(Json::Value &ack, std::string& stream_id)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void SendStreamEvent(const std::string& event, const std::string& stream_id, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);
    virtual void Disconnect(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);

  protected:
    ~SocketSignalingChannel();

  private:
    Json::Value ParseStream(const sio::message::ptr stream);
    sio::client *socket_client_;
    std::vector<std::shared_ptr<ConferenceSignalingChannelObserver>> observers_;
  };
}


#endif
