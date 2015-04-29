/*
 * Intel License
 */


#ifndef WOOGEEN_NATIVE_PEERCONNECTIONHANDLER_H_
#define WOOGEEN_NATIVE_PEERCONNECTIONHANDLER_H_

#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "webrtc/base/messagehandler.h"
#include "webrtc/base/thread.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/jsep.h"

namespace woogeen {
using rtc::Message;
using rtc::Thread;
using rtc::scoped_refptr;
using webrtc::PeerConnectionInterface;
using webrtc::SessionDescriptionInterface;

class PeerConnectionHandlerRequest : public rtc::MessageHandler {
  protected:
    virtual void OnMessage(Message* msg) = 0;

    void SetCallbackThread(Thread* callback_thread) {
      CHECK(callback_thread);
      callback_thread_ = callback_thread;
    }
    Thread* callback_thread_;
};

class CreateSessionDescriptionRequest : public webrtc::CreateSessionDescriptionObserver,
                                        public PeerConnectionHandlerRequest {
  // Declare PeerConnectionHandler as friend class so it can set callback thread.
  friend class PeerConnectionHandler;

  public:
    virtual void OnRequestSuccess(SessionDescriptionInterface* desc) = 0;
    virtual void OnRequestFailure(const std::string& error) = 0;

  protected:
    // Implements CreateSessionDescriptionObserver
    void OnSuccess(SessionDescriptionInterface* desc) override;
    void OnFailure(const std::string& error) override;

    // Implements MessageHandler
    void OnMessage(Message* msg) override;
};

// PeerConnectionHandler is a delegate for the WebRTC PeerConnection API messages going between client SDK and native PeerConnection in libjingle.
// Methods of this class can be called in any thread.
class PeerConnectionHandler : public rtc::MessageHandler {
  public:
    // All operations on PeerConnection will be performed on |pc_thread| and callbacks will be executed on |callback_thread|.
    explicit PeerConnectionHandler(PeerConnectionInterface* peer_connection, Thread* pc_thread, Thread* callback_thread);

    void CreateOffer(CreateSessionDescriptionRequest* request, const webrtc::MediaConstraintsInterface* constrants);

  protected:
    PeerConnectionInterface* peer_connection() {
      return peer_connection_.get();
    }

    // Implements MessageHandler
    void OnMessage(Message* msg) override;

  private:
    Thread* pc_thread_;
    Thread* callback_thread_;
    scoped_refptr<PeerConnectionInterface> peer_connection_;
};
}
#endif // WOOGEEN_NATIVE_PEERCONNECTIONHANDLER_H_
