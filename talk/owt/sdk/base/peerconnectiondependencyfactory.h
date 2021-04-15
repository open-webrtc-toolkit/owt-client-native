// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_PEERCONNECTIONDEPENDENCYFACTORY_H_
#define OWT_BASE_PEERCONNECTIONDEPENDENCYFACTORY_H_
#include <mutex>
#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/api/media_stream_interface.h"
#if defined(WEBRTC_WIN)
#include "webrtc/api/task_queue/task_queue_factory.h"
#include "webrtc/modules/audio_device/win/audio_device_core_win.h"
#endif
#include "webrtc/sdk/media_constraints.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/network.h"
#include "webrtc/p2p/base/basic_packet_socket_factory.h"
namespace owt {
namespace base {
using webrtc::MediaStreamInterface;
using webrtc::AudioDeviceModule;
using webrtc::AudioTrackInterface;
using webrtc::AudioSourceInterface;
using webrtc::VideoTrackInterface;
using webrtc::VideoTrackSourceInterface;
using webrtc::PeerConnectionFactoryInterface;
using webrtc::MediaConstraints;
using rtc::scoped_refptr;
using rtc::Thread;
using rtc::Bind;
// PeerConnectionThread allows blocking calls so other thread can invoke
// synchronized methods on this thread.
class PeerConnectionThread : public rtc::Thread {
 public:
  virtual void Run() override;
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
      webrtc::PeerConnectionObserver* observer);
  rtc::scoped_refptr<MediaStreamInterface> CreateLocalMediaStream(
      const std::string& label);
  rtc::scoped_refptr<AudioTrackInterface> CreateLocalAudioTrack(
      const std::string& id);
  // Make it public to allow passing in user-defined AudioSource.
  rtc::scoped_refptr<AudioTrackInterface> CreateLocalAudioTrack(
      const std::string& id,
      webrtc::AudioSourceInterface* audio_source);
  rtc::scoped_refptr<VideoTrackInterface> CreateLocalVideoTrack(
      const std::string& id,
      webrtc::VideoTrackSourceInterface* video_source);
  rtc::scoped_refptr<AudioSourceInterface> CreateAudioSource(
      const cricket::AudioOptions& options);
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
      webrtc::PeerConnectionObserver* observer);
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  rtc::scoped_refptr<webrtc::AudioDeviceModule> CreateCustomizedAudioDeviceModuleOnCurrentThread();
#endif
  scoped_refptr<PeerConnectionFactoryInterface> pc_factory_;
  static scoped_refptr<PeerConnectionDependencyFactory>
      dependency_factory_;  // Get() always return this instance.
  // This thread performs all operations on pcfactory and pc.
  std::unique_ptr<Thread> pc_thread_;
  // This thread performs all callbacks.
  std::unique_ptr<Thread> callback_thread_;
  std::unique_ptr<rtc::Thread> worker_thread;
  std::unique_ptr<rtc::Thread> signaling_thread;
  std::unique_ptr<rtc::Thread> network_thread;
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  bool render_hardware_acceleration_enabled_;  // Enabling HW acceleration for
                                               // VP8, H.264 & HEVC enc/dec
#endif
  bool encoded_frame_;
  std::string field_trial_;
#if defined(WEBRTC_WIN)
  std::unique_ptr<webrtc::ScopedCOMInitializer> com_initializer_;
  std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
#endif
  std::shared_ptr<rtc::BasicNetworkManager> network_manager_;
  std::shared_ptr<rtc::BasicPacketSocketFactory> packet_socket_factory_;
};
}
}  // namespace owt
#endif  // OWT_BASE_PEERCONNECTIONDEPENDENCYFACTORY_H_
