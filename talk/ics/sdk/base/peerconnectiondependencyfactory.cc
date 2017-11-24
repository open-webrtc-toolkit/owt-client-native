/*
 * Intel License
 */

#include <iostream>
#include "talk/ics/sdk/base/customizedaudiodevicemodule.h"
#include "talk/ics/sdk/base/encodedvideoencoderfactory.h"
#include "talk/ics/sdk/base/peerconnectiondependencyfactory.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/ssladapter.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/media/base/mediachannel.h"
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include "webrtc/system_wrappers/include/field_trial_default.h"
#if defined(WEBRTC_WIN)
#include "talk/ics/sdk/base/win/mftvideodecoderfactory.h"
#include "talk/ics/sdk/base/win/mftvideoencoderfactory.h"
#elif defined(WEBRTC_IOS)
#include "talk/ics/sdk/base/ios/networkmonitorios.h"
#include "talk/ics/sdk/base/objc/ObjcVideoCodecFactory.h"
#endif
#if defined(WEBRTC_LINUX) || defined(WEBRTC_WIN)
#include "talk/ics/sdk/base/customizedvideodecoderfactory.h"
#endif
#include "ics/base/clientconfiguration.h"
#include "ics/base/globalconfiguration.h"

namespace ics {
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

std::mutex PeerConnectionDependencyFactory::get_pc_dependency_factory_mutex_;

PeerConnectionDependencyFactory::PeerConnectionDependencyFactory()
    : pc_thread_(new PeerConnectionThread),
      callback_thread_(new PeerConnectionThread),
      field_trial_("WebRTC-H264HighProfile/Enabled/") {
#if defined(WEBRTC_WIN)
  if (GlobalConfiguration::GetVideoHardwareAccelerationEnabled()) {
    render_hardware_acceleration_enabled_ = true;
    } else {
        render_hardware_acceleration_enabled_ = false;
  }
#endif
#if defined(WEBRTC_IOS)
  network_monitor_ = nullptr;
#endif
  encoded_frame_ = GlobalConfiguration::GetEncodedVideoFrameEnabled();
  pc_thread_->Start();
}

PeerConnectionDependencyFactory::~PeerConnectionDependencyFactory() {}

#if defined(ICS_REBASE_M63)
scoped_refptr<PeerConnectionDependencyFactory>
PeerConnectionDependencyFactory::Create() {
  rtc::RefCountedObject<PeerConnectionDependencyFactory>* pcdf = new
rtc::RefCountedObject<PeerConnectionDependencyFactory>();
  return pcdf;
}
#endif

rtc::scoped_refptr<webrtc::PeerConnectionInterface>
PeerConnectionDependencyFactory::CreatePeerConnection(
    const webrtc::PeerConnectionInterface::RTCConfiguration& config,
    const webrtc::MediaConstraintsInterface* constraints,
    webrtc::PeerConnectionObserver* observer) {
  return pc_thread_
      ->Invoke<scoped_refptr<webrtc::PeerConnectionInterface>>(
          RTC_FROM_HERE, Bind(&PeerConnectionDependencyFactory::
                                  CreatePeerConnectionOnCurrentThread,
                              this, config, constraints, observer))
      .get();
}

PeerConnectionDependencyFactory* PeerConnectionDependencyFactory::Get() {
  std::lock_guard<std::mutex> lock(get_pc_dependency_factory_mutex_);
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
  LOG(LS_INFO) << "CreatePeerConnectionOnCurrentThread";

  webrtc::field_trial::InitFieldTrialsFromString(field_trial_.c_str());

  if (!rtc::InitializeSSL()) {
    LOG(LS_ERROR) << "Failed to initialize SSL.";
    RTC_NOTREACHED();
    return;
  }

  rtc::Thread* worker_thread = new rtc::Thread();
  worker_thread->SetName("worker_thread", NULL);
  rtc::Thread* signaling_thread = new rtc::Thread();
  signaling_thread->SetName("signaling_thread", NULL);
  RTC_CHECK(worker_thread->Start() && signaling_thread->Start())
      << "Failed to start threads";
  std::unique_ptr<cricket::WebRtcVideoEncoderFactory> encoder_factory;
  std::unique_ptr<cricket::WebRtcVideoDecoderFactory> decoder_factory;

#if defined(WEBRTC_IOS)
  encoder_factory = ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory();
  decoder_factory = ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory();
#endif

#if defined(ICS_REBASE_M63)
  if (GlobalConfiguration::GetCustomizedVideoDecoderEnabled())
    decoder_factory.reset(new CustomizedVideoDecoderFactory(GlobalConfiguration::GetCustomizedVideoDecoder()));
  else {
#if defined(WEBRTC_WIN)
    if (render_hardware_acceleration_enabled_) {
      encoder_factory.reset(new MSDKVideoEncoderFactory());
      decoder_factory.reset(new MSDKVideoDecoderFactory());
    }
#endif
  }
#endif
  // Encoded video frame enabled
  if (encoded_frame_)
  {
    encoder_factory.reset(new EncodedVideoEncoderFactory());
  }
  // Raw audio frame
  // if adm is nullptr, voe_base will initilize it with the default internal
  // adm.
  rtc::scoped_refptr<AudioDeviceModule> adm;
  if (GlobalConfiguration::GetCustomizedAudioInputEnabled()) {
    // Create ADM on worker thred as RegisterAudioCallback is invoked there.
    adm = worker_thread->Invoke<rtc::scoped_refptr<AudioDeviceModule>>(
               RTC_FROM_HERE,
               Bind(&PeerConnectionDependencyFactory::
                    CreateCustomizedAudioDeviceModuleOnCurrentThread,
                    this));
  }

  pc_factory_ = webrtc::CreatePeerConnectionFactory(
      worker_thread, signaling_thread, nullptr,
      encoder_factory.release(),   // Encoder factory
      decoder_factory.release());  // Decoder factory

  LOG(LS_INFO) << "CreatePeerConnectionOnCurrentThread finished.";
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
  pc_thread_->Invoke<void>(RTC_FROM_HERE,
                           Bind(&PeerConnectionDependencyFactory::
                                    CreatePeerConnectionFactoryOnCurrentThread,
                                this));
  RTC_CHECK(pc_factory_.get());
}

scoped_refptr<webrtc::MediaStreamInterface>
PeerConnectionDependencyFactory::CreateLocalMediaStream(
    const std::string& label) {
  RTC_CHECK(pc_thread_);
  return pc_thread_->Invoke<scoped_refptr<webrtc::MediaStreamInterface>>(
      RTC_FROM_HERE,
      Bind(&PeerConnectionFactoryInterface::CreateLocalMediaStream,
           pc_factory_.get(), label));
}

scoped_refptr<webrtc::VideoTrackSourceInterface>
PeerConnectionDependencyFactory::CreateVideoSource(
    std::unique_ptr<cricket::VideoCapturer> capturer,
    const MediaConstraintsInterface* constraints) {
  return pc_thread_
      ->Invoke<scoped_refptr<webrtc::VideoTrackSourceInterface>>(
          RTC_FROM_HERE,
          Bind((rtc::scoped_refptr<VideoTrackSourceInterface>(
                   PeerConnectionFactoryInterface::*)(
                   cricket::VideoCapturer*,
                   const MediaConstraintsInterface*)) &
                   PeerConnectionFactoryInterface::CreateVideoSource,
               pc_factory_.get(), capturer.release(), constraints))
      .get();
}

scoped_refptr<VideoTrackInterface>
PeerConnectionDependencyFactory::CreateLocalVideoTrack(
    const std::string& id,
    webrtc::VideoTrackSourceInterface* video_source) {
  return pc_thread_->Invoke<scoped_refptr<VideoTrackInterface>>(RTC_FROM_HERE,
                       Bind(&PeerConnectionFactoryInterface::CreateVideoTrack,
                            pc_factory_.get(), id, video_source))
      .get();
}

scoped_refptr<AudioTrackInterface>
PeerConnectionDependencyFactory::CreateLocalAudioTrack(const std::string& id) {
  bool aec_enabled, agc_enabled, ns_enabled;
  aec_enabled = GlobalConfiguration::GetAECEnabled();
  agc_enabled = GlobalConfiguration::GetAGCEnabled();
  ns_enabled = GlobalConfiguration::GetNSEnabled();
  if (!aec_enabled || !agc_enabled || !ns_enabled) {
    cricket::AudioOptions options;
    options.echo_cancellation = rtc::Optional<bool>(aec_enabled ? true : false);
    options.auto_gain_control = rtc::Optional<bool>(agc_enabled ? true : false);
    options.noise_suppression = rtc::Optional<bool>(ns_enabled ? true : false);
    options.residual_echo_detector =
        rtc::Optional<bool>(aec_enabled ? true : false);
    scoped_refptr<webrtc::AudioSourceInterface> audio_source =
        CreateAudioSource(options);
    return pc_thread_
        ->Invoke<scoped_refptr<AudioTrackInterface>>(
            RTC_FROM_HERE,
            Bind(&PeerConnectionFactoryInterface::CreateAudioTrack,
                 pc_factory_.get(), id, audio_source.get()))
        .get();
  } else {
    return pc_thread_
        ->Invoke<scoped_refptr<AudioTrackInterface>>(
            RTC_FROM_HERE,
            Bind(&PeerConnectionFactoryInterface::CreateAudioTrack,
                 pc_factory_.get(), id, nullptr))
        .get();
  }
}

scoped_refptr<AudioTrackInterface>
PeerConnectionDependencyFactory::CreateLocalAudioTrack(
    const std::string& id,
    webrtc::AudioSourceInterface* audio_source) {
  return pc_thread_
      ->Invoke<scoped_refptr<AudioTrackInterface>>(
          RTC_FROM_HERE, Bind(&PeerConnectionFactoryInterface::CreateAudioTrack,
                              pc_factory_.get(), id, audio_source))
      .get();
}

rtc::scoped_refptr<AudioSourceInterface>
PeerConnectionDependencyFactory::CreateAudioSource(
    const cricket::AudioOptions& options) {
  return pc_thread_
      ->Invoke<scoped_refptr<webrtc::AudioSourceInterface>>(
          RTC_FROM_HERE,
          Bind((rtc::scoped_refptr<AudioSourceInterface>(
                   PeerConnectionFactoryInterface::*)(
                   const cricket::AudioOptions&)) &
                   PeerConnectionFactoryInterface::CreateAudioSource,
               pc_factory_.get(), options))
      .get();
}

rtc::scoped_refptr<PeerConnectionFactoryInterface>
PeerConnectionDependencyFactory::PeerConnectionFactory() const {
  return pc_factory_;
}

rtc::NetworkMonitorInterface* PeerConnectionDependencyFactory::NetworkMonitor(){
#if defined(WEBRTC_IOS)
  pc_thread_->Invoke<void>(
      RTC_FROM_HERE,
      Bind(
          &PeerConnectionDependencyFactory::CreateNetworkMonitorOnCurrentThread,
          this));
  return network_monitor_;
#else
  return nullptr;
#endif
}

void PeerConnectionDependencyFactory::CreateNetworkMonitorOnCurrentThread() {
#if defined(WEBRTC_IOS)
  if (!network_monitor_) {
     network_monitor_ = new NetworkMonitorIos();
     network_monitor_->Start();
  }
#endif
}

scoped_refptr<webrtc::AudioDeviceModule>
PeerConnectionDependencyFactory::CreateCustomizedAudioDeviceModuleOnCurrentThread() {
  return CustomizedAudioDeviceModule::Create(
     GlobalConfiguration::GetAudioFrameGenerator());
}

}
}
