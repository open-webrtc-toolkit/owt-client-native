/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_PEERCONNECTIONDEPENDENCYFACTORY_H_
#define WOOGEEN_BASE_PEERCONNECTIONDEPENDENCYFACTORY_H_

#include <mutex>
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/base/bind.h"

namespace woogeen {
namespace base {
using webrtc::MediaStreamInterface;
using webrtc::AudioTrackInterface;
using webrtc::VideoTrackInterface;
using webrtc::VideoTrackSourceInterface;
using webrtc::PeerConnectionFactoryInterface;
using webrtc::MediaConstraintsInterface;
using rtc::scoped_refptr;
using rtc::Thread;
using rtc::Bind;

// PeerConnectionThread allows blocking calls so other thread can invoke
// synchronized methods on this thread.
class PeerConnectionThread : public rtc::Thread {
 public:
  virtual void Run();
  ~PeerConnectionThread() override;
};

// Object factory for WebRTC PeerConnections.
class PeerConnectionDependencyFactory : public rtc::RefCountInterface {
 public:
  // Get a PeerConnectionDependencyFactory instance. It doesn't create a new
  // instance. It always return the same instance.
  static PeerConnectionDependencyFactory* Get();
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> CreatePeerConnection(
      const webrtc::PeerConnectionInterface::RTCConfiguration& config,
      const webrtc::MediaConstraintsInterface* constraints,
      webrtc::PeerConnectionObserver* observer);
  rtc::scoped_refptr<MediaStreamInterface> CreateLocalMediaStream(
      const std::string& label);
  rtc::scoped_refptr<AudioTrackInterface> CreateLocalAudioTrack(
      const std::string& id);
  rtc::scoped_refptr<VideoTrackInterface> CreateLocalVideoTrack(
      const std::string& id,
      webrtc::VideoTrackSourceInterface* video_source);
  rtc::scoped_refptr<VideoTrackSourceInterface> CreateVideoSource(
      cricket::VideoCapturer* capturer,
      const MediaConstraintsInterface* constraints);
  rtc::NetworkMonitorInterface* NetworkMonitor();
  // Returns current |pc_factory_|.
  rtc::scoped_refptr<PeerConnectionFactoryInterface> PeerConnectionFactory()
      const;

  ~PeerConnectionDependencyFactory();
 protected:
  explicit PeerConnectionDependencyFactory();
  virtual const rtc::scoped_refptr<PeerConnectionFactoryInterface>&
  GetPeerConnectionFactory();

 private:
  // Create a PeerConnectionDependencyFactory instance.
  // static rtc::scoped_refptr<PeerConnectionDependencyFactory> Create();
  void CreatePeerConnectionFactory();
  void CreatePeerConnectionFactoryOnCurrentThread();
  rtc::scoped_refptr<webrtc::PeerConnectionInterface>
  CreatePeerConnectionOnCurrentThread(
      const webrtc::PeerConnectionInterface::RTCConfiguration& config,
      const webrtc::MediaConstraintsInterface* constraints,
      webrtc::PeerConnectionObserver* observer);
  void CreateNetworkMonitorOnCurrentThread();

  scoped_refptr<PeerConnectionFactoryInterface> pc_factory_;
  static scoped_refptr<PeerConnectionDependencyFactory>
      dependency_factory_;  // Get() always return this instance.
  Thread*
      pc_thread_;  // This thread performs all operations on pcfactory and pc.
  Thread* callback_thread_;  // This thread performs all callbacks.
#if defined(WEBRTC_WIN)
  bool render_hardware_acceleration_enabled_;  // Enabling HW acceleration for
                                               // VP8, H.264 & HEVC enc/dec
#endif
  bool encoded_frame_;
  static std::mutex get_pc_dependency_factory_mutex_;
  rtc::NetworkMonitorInterface* network_monitor_;
};
}
}  // namespace woogeen
#endif  // WOOGEEN_BASE_PEERCONNECTIONDEPENDENCYFACTORY_H_
