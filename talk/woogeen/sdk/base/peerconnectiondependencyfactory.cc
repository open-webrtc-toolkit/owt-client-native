/*
 * Intel License
 */

#include "webrtc/base/thread.h"
#include "webrtc/base/bind.h"
#include "webrtc/base/ssladapter.h"
#include "peerconnectiondependencyfactory.h"

namespace woogeen {
void PeerConnectionThread::Run() {
  ProcessMessages(kForever);
  SetAllowBlockingCalls(true);
}

PeerConnectionThread::~PeerConnectionThread() {
  Stop();
}

scoped_refptr<PeerConnectionDependencyFactory> PeerConnectionDependencyFactory::dependency_factory_;

PeerConnectionDependencyFactory::PeerConnectionDependencyFactory()
  : pc_thread_(new PeerConnectionThread),
    callback_thread_(new PeerConnectionThread) {
  pc_thread_->Start();
  this->CreatePeerConnectionFactory();
}

PeerConnectionDependencyFactory::~PeerConnectionDependencyFactory() {
}

/*
scoped_refptr<PeerConnectionDependencyFactory> PeerConnectionDependencyFactory::Create() {
  rtc::RefCountedObject<PeerConnectionDependencyFactory>* pcdf = new rtc::RefCountedObject<PeerConnectionDependencyFactory>();
  return pcdf;
}*/

rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionDependencyFactory::CreatePeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config, const webrtc::MediaConstraintsInterface* constraints, webrtc::PeerConnectionObserver* observer) {
  return pc_thread_->Invoke<scoped_refptr<webrtc::PeerConnectionInterface>>(Bind(&PeerConnectionDependencyFactory::CreatePeerConnectionOnCurrentThread, this, config, constraints, observer)).get();
}

PeerConnectionDependencyFactory* PeerConnectionDependencyFactory::Get() {
  if (!dependency_factory_.get())
    dependency_factory_ = new rtc::RefCountedObject<PeerConnectionDependencyFactory>();
  return dependency_factory_.get();
}

const scoped_refptr<PeerConnectionFactoryInterface>& PeerConnectionDependencyFactory::GetPeerConnectionFactory() {
  if (!pc_factory_.get())
    CreatePeerConnectionFactory();
  CHECK(pc_factory_.get());
  return pc_factory_;
}

void PeerConnectionDependencyFactory::CreatePeerConnectionFactoryOnCurrentThread() {
  if (!rtc::InitializeSSL()) {
    LOG(LS_ERROR) << "Failed to initialize SSL.";
  }
  pc_factory_ = webrtc::CreatePeerConnectionFactory();
}

scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionDependencyFactory::CreatePeerConnectionOnCurrentThread(const webrtc::PeerConnectionInterface::RTCConfiguration& config, const webrtc::MediaConstraintsInterface* constraints, webrtc::PeerConnectionObserver* observer) {
  return (pc_factory_->CreatePeerConnection(config, constraints, nullptr, nullptr, observer)).get();
}

void PeerConnectionDependencyFactory::CreatePeerConnectionFactory() {
  DCHECK(!pc_factory_.get());
  LOG(LS_INFO) << "PeerConnectionDependencyFactory::CreatePeerConnectionFactory()";
  CHECK(pc_thread_);
  pc_thread_->Invoke<void>(Bind(&PeerConnectionDependencyFactory::CreatePeerConnectionFactoryOnCurrentThread, this));
  CHECK(pc_factory_.get());
}

scoped_refptr<webrtc::MediaStreamInterface> PeerConnectionDependencyFactory::CreateLocalMediaStream(const std::string &label) {
  CHECK(pc_thread_);
  return pc_thread_->Invoke<scoped_refptr<webrtc::MediaStreamInterface>>(Bind(&PeerConnectionFactoryInterface::CreateLocalMediaStream, pc_factory_.get(), label)).get();
}

scoped_refptr<webrtc::VideoSourceInterface> PeerConnectionDependencyFactory::CreateVideoSource(cricket::VideoCapturer* capturer, const MediaConstraintsInterface* constraints) {
  return pc_thread_->Invoke<scoped_refptr<webrtc::VideoSourceInterface>>(Bind(&PeerConnectionFactoryInterface::CreateVideoSource, pc_factory_.get(), capturer, constraints)).get();
}

scoped_refptr<VideoTrackInterface> PeerConnectionDependencyFactory::CreateLocalVideoTrack(const std::string &id, webrtc::VideoSourceInterface* video_source) {
  return pc_thread_->Invoke<scoped_refptr<VideoTrackInterface>>(Bind(&PeerConnectionFactoryInterface::CreateVideoTrack, pc_factory_.get(), id, video_source)).get();
}

scoped_refptr<webrtc::MediaStreamInterface> PeerConnectionDependencyFactory::CreateLocalMediaStream(const std::string &label, cricket::VideoCapturer* capturer, const MediaConstraintsInterface* constraints) {
  scoped_refptr<MediaStreamInterface> stream = this->CreateLocalMediaStream(label);
  scoped_refptr<VideoSourceInterface> source = this->CreateVideoSource(capturer, constraints);
  std::string video_track_label = "VideoTrack";
  scoped_refptr<VideoTrackInterface> video_track = this->CreateLocalVideoTrack(video_track_label, source);
  stream->AddTrack(video_track);
  return stream;
}
}
