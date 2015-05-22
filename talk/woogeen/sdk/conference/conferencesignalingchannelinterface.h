/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_SIGNALINGCHANNELINTERFACE_H_
#define WOOGEEN_CONFERENCE_SIGNALINGCHANNELINTERFACE_H_

#include <functional>
#include "talk/woogeen/sdk/conference/conferenceexception.h"

namespace woogeen {

class ConferenceSignalingChannelInterface {
  public:
    virtual void Connect(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure) = 0;
};

}

#endif // WOOGEEN_CONFERENCE_SIGNALINGCHANNELINTERFACE_H_
