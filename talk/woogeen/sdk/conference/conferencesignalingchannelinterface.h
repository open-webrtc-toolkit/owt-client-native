/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_SIGNALINGCHANNELINTERFACE_H_
#define WOOGEEN_CONFERENCE_SIGNALINGCHANNELINTERFACE_H_

#include <functional>
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "webrtc/base/json.h"

namespace woogeen {

class ConferenceSignalingChannelInterface {
  public:
    virtual void Connect(const std::string& token, std::function<void(Json::Value &room_info)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) = 0;
    virtual void Publish(Json::Value &options, std::string &sdp, std::function<void(Json::Value &ack)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) = 0;
};

}

#endif // WOOGEEN_CONFERENCE_SIGNALINGCHANNELINTERFACE_H_
