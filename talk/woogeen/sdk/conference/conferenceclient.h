/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_

#include <memory>
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"

namespace woogeen {

class ConferenceClient {
  public:
    ConferenceClient(std::unique_ptr<ConferenceSignalingChannelInterface> signaling_channel);
    void Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  private:
    std::unique_ptr<ConferenceSignalingChannelInterface> signaling_channel_;
};

}


#endif // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
