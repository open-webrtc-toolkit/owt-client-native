/*
 * Intel License
 */

#include <iostream>
#include "webrtc/base/thread.h"
#include "webrtc/base/bind.h"
#include "webrtc/base/ssladapter.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/base/encodedvideoencoderfactory.h"
#if defined(WEBRTC_WIN)
#include "talk/woogeen/sdk/base/win/mftvideodecoderfactory.h"
#include "talk/woogeen/sdk/base/win/mftvideoencoderfactory.h"
#endif
#if defined(WEBRTC_LINUX)
#include "talk/woogeen/sdk/base/linux/v4l2videodecoderfactory.h"
#endif
#include "woogeen/base/clientconfiguration.h"
#include "woogeen/base/globalconfiguration.h"

namespace woogeen {
namespace base {

void PeerConnectionThread::Run() {
  ProcessMessages(kForever);
  SetAllowBlockingCalls(true);
}

PeerConnectionThread::~PeerConnectionThread() {
  LOG(LS_INFO) << "Quit a PeerConnectionThread.";
  Stop();
}

rtc::scoped_refptr<PeerConnectionDependencyFactory>
    PeerConnectionDependencyFactory::dependency_factory_;

PeerConnectionDependencyFactory::PeerConnectionDependencyFactory()
    : pc_thread_(new PeerConnectionThread),
      callback_thread_(new PeerConnectionThread){
#if defined(WEBRTC_WIN)
  if (GlobalConfiguration::GetCodecHardwareAccelerationEnabled() && (GlobalConfiguration::GetRenderWindow() != nullptr)){
        render_hardware_acceleration_enabled_ = true;
        render_window_ = GlobalConfiguration::GetRenderWindow();
    }else{
        render_hardware_acceleration_enabled_ = false;
        render_window_ = nullptr;
  }
#endif
  encoded_frame_ = GlobalConfiguration::GetEncodedVideoFrameEnabled();
  pc_thread_->Start();
}

PeerConnectionDependencyFactory::~PeerConnectionDependencyFactory() {}

/*
scoped_refptr<PeerConnectionDependencyFactory>
PeerConnectionDependencyFactory::Create() {
  rtc::RefCountedObject<PeerConnectionDependencyFactory>* pcdf = new
rtc::RefCountedObject<PeerConnectionDependencyFactory>();
  return pcdf;
}*/

rtc::scoped_refptr<webrtc::PeerConnectionInterface>
PeerConnectionDependencyFactory::CreatePeerConnection(
    const webrtc::PeerConnectionInterface::RTCConfiguration& config,
    const webrtc::MediaConstraintsInterface* constraints,
    webrtc::PeerConnectionObserver* observer) {
  return pc_thread_
      ->Invoke<scoped_refptr<webrtc::PeerConnectionInterface>>(Bind(
          &PeerConnectionDependencyFactory::CreatePeerConnectionOnCurrentThread,
          this, config, constraints, observer))
      .get();
}

PeerConnectionDependencyFactory* PeerConnectionDependencyFactory::Get() {
  if (!dependency_factory_.get()) {
    dependency_factory_ =
        new rtc::RefCountedObject<PeerConnectionDependencyFactory>();
    dependency_factory_->CreatePeerConnectionFactory();
  }
  return dependency_factory_.get();
}

const scoped_refptr<PeerConnectionFactoryInterface>&
PeerConnectionDependencyFactory::GetPeerConnectionFactory() {
  if (!pc_factory_.get())
    CreatePeerConnectionFactory();
  RTC_CHECK(pc_factory_.get());
  return pc_factory_;
}

void PeerConnectionDependencyFactory::
    CreatePeerConnectionFactoryOnCurrentThread() {
  if (!rtc::InitializeSSL()) {
    LOG(LS_ERROR) << "Failed to initialize SSL.";
  }
  rtc::Thread* worker_thread = new rtc::Thread();
  worker_thread->SetName("worker_thread", NULL);
  rtc::Thread* signaling_thread = new rtc::Thread();
  signaling_thread->SetName("signaling_thread", NULL);
  RTC_CHECK(worker_thread->Start() && signaling_thread->Start())
      << "Failed to start threads";
  if (encoded_frame_) {
     rtc::scoped_ptr<cricket::WebRtcVideoEncoderFactory> encoder_factory;
     encoder_factory.reset(new EncodedVideoEncoderFactory());
#if defined(WEBRTC_WIN)
     rtc::scoped_ptr<cricket::WebRtcVideoDecoderFactory> decoder_factory;
     decoder_factory.reset(new MSDKVideoDecoderFactory(nullptr));
#elif defined(WEBRTC_LINUX)
     rtc::scoped_ptr<cricket::WebRtcVideoDecoderFactory> decoder_factory;
     decoder_factory.reset(new V4L2VideoDecoderFactory());
#endif
     pc_factory_ = webrtc::CreatePeerConnectionFactory(worker_thread,
          signaling_thread,
          NULL, //Default ADM
          encoder_factory.release(), //Encoder factory
  //TODO: If iOS to support encoded frame, needs to
  //implement decoder_factory for iOS.
#if defined(WEBRTC_IOS)
          NULL); //Decoder factory
#else
          decoder_factory.release()); //Decoder factory
#endif
     return;
  }

#if defined(WEBRTC_WIN)
  if (render_hardware_acceleration_enabled_ && render_window_ != nullptr){
      //We create peer connection factory with dedicated decoder factory.
      rtc::scoped_ptr<cricket::WebRtcVideoDecoderFactory> decoder_factory;
      rtc::scoped_ptr<cricket::WebRtcVideoEncoderFactory> encoder_factory;
      decoder_factory.reset(new MSDKVideoDecoderFactory(render_window_));
      encoder_factory.reset(new MSDKVideoEncoderFactory());
      pc_factory_ = webrtc::CreatePeerConnectionFactory(worker_thread,
          signaling_thread,
          NULL, //Default ADM
          encoder_factory.release(), //Encoder factory
          decoder_factory.release());
  }else
#endif
      pc_factory_ = webrtc::CreatePeerConnectionFactory(worker_thread,
          signaling_thread,
          NULL, //Default ADM
          NULL, //Encoder factory
          NULL);
}

scoped_refptr<webrtc::PeerConnectionInterface>
PeerConnectionDependencyFactory::CreatePeerConnectionOnCurrentThread(
    const webrtc::PeerConnectionInterface::RTCConfiguration& config,
    const webrtc::MediaConstraintsInterface* constraints,
    webrtc::PeerConnectionObserver* observer) {
  return (pc_factory_->CreatePeerConnection(config, constraints, nullptr,
                                            nullptr, observer))
      .get();
}

void PeerConnectionDependencyFactory::CreatePeerConnectionFactory() {
  RTC_CHECK(!pc_factory_.get());
  LOG(LS_INFO)
      << "PeerConnectionDependencyFactory::CreatePeerConnectionFactory()";
  RTC_CHECK(pc_thread_);
  pc_thread_->Invoke<void>(Bind(&PeerConnectionDependencyFactory::
                                    CreatePeerConnectionFactoryOnCurrentThread,
                                this));
  RTC_CHECK(pc_factory_.get());
}

scoped_refptr<webrtc::MediaStreamInterface>
PeerConnectionDependencyFactory::CreateLocalMediaStream(
    const std::string& label) {
  RTC_CHECK(pc_thread_);
  return pc_thread_->Invoke<scoped_refptr<webrtc::MediaStreamInterface>>(
      Bind(&PeerConnectionFactoryInterface::CreateLocalMediaStream,
           pc_factory_.get(), label));
}

scoped_refptr<webrtc::VideoSourceInterface>
PeerConnectionDependencyFactory::CreateVideoSource(
    cricket::VideoCapturer* capturer,
    const MediaConstraintsInterface* constraints) {
  return pc_thread_->Invoke<scoped_refptr<webrtc::VideoSourceInterface>>(
                       Bind(&PeerConnectionFactoryInterface::CreateVideoSource,
                            pc_factory_.get(), capturer, constraints))
      .get();
}

scoped_refptr<VideoTrackInterface>
PeerConnectionDependencyFactory::CreateLocalVideoTrack(
    const std::string& id,
    webrtc::VideoSourceInterface* video_source) {
  return pc_thread_->Invoke<scoped_refptr<VideoTrackInterface>>(
                       Bind(&PeerConnectionFactoryInterface::CreateVideoTrack,
                            pc_factory_.get(), id, video_source))
      .get();
}

scoped_refptr<AudioTrackInterface>
PeerConnectionDependencyFactory::CreateLocalAudioTrack(const std::string& id) {
  return pc_thread_->Invoke<scoped_refptr<AudioTrackInterface>>(
                       Bind(&PeerConnectionFactoryInterface::CreateAudioTrack,
                            pc_factory_.get(), id, nullptr))
      .get();
}
}
}
