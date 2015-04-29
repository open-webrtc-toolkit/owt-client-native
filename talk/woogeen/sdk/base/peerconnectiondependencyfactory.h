/*
 * Intel License
 */

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "webrtc/base/bind.h"
#include "peerconnectionhandler.h"

#ifndef WOOGEEN_NATIVE_PEERCONNECTIONDEPENDENCYFACTORY_H_
#define WOOGEEN_NATIVE_PEERCONNECTIONDEPENDENCYFACTORY_H_

namespace woogeen {
using webrtc::MediaStreamInterface;
using webrtc::AudioTrackInterface;
using webrtc::VideoTrackInterface;
using webrtc::VideoSourceInterface;
using webrtc::PeerConnectionFactoryInterface;
using webrtc::MediaConstraintsInterface;
using rtc::scoped_refptr;
using rtc::Thread;
using rtc::Bind;

// PeerConnectionThread allows blocking calls so other thread can invoke synchronized methods on this thread.
class PeerConnectionThread : public rtc::Thread {
  public:
    virtual void Run();
    ~PeerConnectionThread() override;
};

// Object factory for WebRTC PeerConnections.
class PeerConnectionDependencyFactory : public rtc::RefCountInterface {
  public:
    static rtc::scoped_refptr<PeerConnectionDependencyFactory> Create();
    PeerConnectionHandler* CreatePeerConnectionHandler();
    rtc::scoped_refptr<MediaStreamInterface> CreateLocalMediaStream(const std::string &label, cricket::VideoCapturer* capturer, const MediaConstraintsInterface* constraints);
  protected:
    explicit PeerConnectionDependencyFactory();
    ~PeerConnectionDependencyFactory();
    virtual const rtc::scoped_refptr<PeerConnectionFactoryInterface>& GetPeerConnectionFactory();

    rtc::scoped_refptr<MediaStreamInterface> CreateLocalMediaStream(const std::string &label);
    rtc::scoped_refptr<AudioTrackInterface> CreateLocalAudioTrack(const std::string &id);
    rtc::scoped_refptr<VideoTrackInterface> CreateLocalVideoTrack(const std::string &id, webrtc::VideoSourceInterface* video_source);
    rtc::scoped_refptr<VideoSourceInterface> CreateVideoSource(cricket::VideoCapturer* capturer, const MediaConstraintsInterface* constraints);

  private:
    void CreatePeerConnectionFactory();
    void CreatePeerConnectionFactoryOnCurrentThread();

    scoped_refptr<PeerConnectionFactoryInterface> pc_factory_;
    Thread* pc_thread_;  // This thread performs all operations on pcfactory and pc.
    Thread* callback_thread_;  // This thread performs all callbacks.
};
}  // namespace woogeen
#endif  // WOOGEEN_NATIVE_PEERCONNECTIONDEPENDENCYFACTORY_H_
