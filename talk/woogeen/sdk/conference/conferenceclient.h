/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
#define WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include "talk/woogeen/sdk/conference/conferenceexception.h"
#include "talk/woogeen/sdk/conference/conferencesignalingchannelinterface.h"
#include "talk/woogeen/sdk/conference/conferencepeerconnectionchannel.h"
#include "talk/woogeen/sdk/conference/remotemixedstream.h"

namespace woogeen {

struct ConferenceClientConfiguration {
  std::vector<webrtc::PeerConnectionInterface::IceServer> ice_servers;
};

class ConferenceClientObserver {
  public:
    // Triggered when a new stream is added.
    virtual void OnStreamAdded(std::shared_ptr<woogeen::RemoteCameraStream> stream) {};
    virtual void OnStreamAdded(std::shared_ptr<woogeen::RemoteScreenStream> stream) {};
    virtual void OnStreamAdded(std::shared_ptr<woogeen::RemoteMixedStream> stream) {};
    // Triggered when a remote stream is removed.
    virtual void OnStreamRemoved(std::shared_ptr<woogeen::RemoteCameraStream> stream) {};
    virtual void OnStreamRemoved(std::shared_ptr<woogeen::RemoteScreenStream> stream) {};
    virtual void OnStreamRemoved(std::shared_ptr<woogeen::RemoteMixedStream> stream) {};
    // TODO(jianjun): add other events.
};

class ConferenceClient final : ConferenceSignalingChannelObserver{
  public:
    ConferenceClient(ConferenceClientConfiguration& configuration, std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel);
    // Add an observer for conferenc client.
    void AddObserver(std::shared_ptr<ConferenceClientObserver> observer);
    // Remove an object from conference client.
    void RemoveObserver(std::shared_ptr<ConferenceClientObserver> observer);
    // Join a conference.
    void Join(const std::string& token, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Publish a stream to the conference.
    void Publish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Subscribe a stream from the conference.
    void Subscribe(std::shared_ptr<RemoteStream> stream, std::function<void(std::shared_ptr<RemoteStream> stream)> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Unpublish a stream to the conference.
    void Unpublish(std::shared_ptr<LocalStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Unsubscribe a stream from the conference.
    void Unsubscribe(std::shared_ptr<RemoteStream> stream, std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
    // Leave this conference.
    void Leave(std::function<void()> on_success, std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

  protected:
    virtual void OnStreamAdded(Json::Value stream);

  private:
    bool CheckNullPointer(uintptr_t pointer, std::function<void(std::unique_ptr<ConferenceException>)>on_failure);
    void TriggerOnStreamAdded(const Json::Value& stream_info);

    ConferenceClientConfiguration& configuration_;
    std::shared_ptr<ConferenceSignalingChannelInterface> signaling_channel_;
    std::unordered_map<std::string, std::shared_ptr<ConferencePeerConnectionChannel>> publish_pcs_;
    std::unordered_map<std::string, std::shared_ptr<ConferencePeerConnectionChannel>> subscribe_pcs_;
    std::vector<std::shared_ptr<ConferenceClientObserver>> observers_;
};

}


#endif // WOOGEEN_CONFERENCE_CONFERENCECLIENT_H_
