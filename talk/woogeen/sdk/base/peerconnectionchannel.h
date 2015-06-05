/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_PEERCONNECTIONCHANNEL_H_
#define WOOGEEN_BASE_PEERCONNECTIONCHANNEL_H_

#include "webrtc/base/messagehandler.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/base/mediaconstraintsimpl.h"
#include "talk/woogeen/sdk/base/functionalobserver.h"

namespace woogeen {

using webrtc::PeerConnectionInterface;

struct SetSessionDescriptionMessage : public rtc::MessageData {
  explicit SetSessionDescriptionMessage(FunctionalSetSessionDescriptionObserver* observer, webrtc::SessionDescriptionInterface* desc)
      : observer(observer),
        description(desc) {
  }

  rtc::scoped_refptr<FunctionalSetSessionDescriptionObserver> observer;
  webrtc::SessionDescriptionInterface* description;
};

class PeerConnectionChannel : public rtc::MessageHandler,
                              public webrtc::PeerConnectionObserver {
  public:
    PeerConnectionChannel();

  protected:
    virtual ~PeerConnectionChannel(){};
    bool InitializePeerConnection();
    const webrtc::SessionDescriptionInterface* LocalDescription();

    // Subclasses should prepare observers for these two functions and post message to PeerConnectionChannel.
    virtual void CreateOffer() = 0;
    virtual void CreateAnswer() = 0;

    virtual void OnMessage(rtc::Message* msg);

    // PeerConnectionObserver
    virtual void OnSignalingChange(PeerConnectionInterface::SignalingState new_state);
    virtual void OnAddStream(MediaStreamInterface* stream);
    virtual void OnRemoveStream(MediaStreamInterface* stream);
    virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel);
    virtual void OnRenegotiationNeeded();
    virtual void OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state);
    virtual void OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state);
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

    // CreateSessionDescriptionObserver
    virtual void OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc);
    virtual void OnCreateSessionDescriptionFailure(const std::string& error);

    // SetSessionDescriptionObserver
    virtual void OnSetLocalSessionDescriptionSuccess();
    virtual void OnSetLocalSessionDescriptionFailure(const std::string& error);
    virtual void OnSetRemoteSessionDescriptionSuccess();
    virtual void OnSetRemoteSessionDescriptionFailure(const std::string& error);

    enum MessageType : int {
      kMessageTypeCreateOffer = 101,
      kMessageTypeCreateAnswer,
      kMessageTypeSetLocalDescription,
      kMessageTypeSetRemoteDescription,
      kMessageTypeAddStream,
      kMessageTypeRemoveStream,
      kMessageTypeClosePeerConnection,
    };

    Thread* pc_thread_;

  private:
    // |factory_| is got from PeerConnectionDependencyFactory::Get() which is shared among all PeerConnectionChannels.
    rtc::scoped_refptr<woogeen::PeerConnectionDependencyFactory> factory_;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    woogeen::MediaConstraintsImpl media_constraints_;
};

}

#endif // WOOGEEN_BASE_PEERCONNECTIONCHANNEL_H_
