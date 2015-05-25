/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_

#include <memory>
#include <unordered_map>
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"

namespace woogeen {

class ConferenceClient {
  public:
    ConferenceClient(std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel);
    // Join a conference.
    void Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Publish a stream to the conference.
    void Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Subscribe a stream from the conference.
    void Subscribe(std::shared_ptr<RemoteStream> stream, std::function<void(std::shared_ptr<RemoteStream> stream)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  private:
    bool CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);

    std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel_;
    std::unordered_map<std::string, std::shared_ptr<ConferencePeerConnectionChannel>> publish_pcs_;
    std::unordered_map<std::string, std::shared_ptr<ConferencePeerConnectionChannel>> subscribe_pcs_;
};

}


#endif // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
