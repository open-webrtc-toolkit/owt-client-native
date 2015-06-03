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
    virtual void Connect(const std::string& token, std::function<void(Json::Value &room_info)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    virtual void Publish(Json::Value &options, std::string &sdp, std::function<void(Json::Value &ack)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  private:
    sio::client *socket_client_;
  };
}


#endif
