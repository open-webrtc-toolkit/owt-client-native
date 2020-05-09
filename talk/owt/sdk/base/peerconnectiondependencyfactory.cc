// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <iostream>
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
#include "talk/owt/sdk/base/customizedaudiodevicemodule.h"
#endif
#include "talk/owt/sdk/base/encodedvideoencoderfactory.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#include "webrtc/api/audio_codecs/builtin_audio_decoder_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_encoder_factory.h"
#include "webrtc/api/create_peerconnection_factory.h"
#include "webrtc/api/video_codecs/builtin_video_decoder_factory.h"
#include "webrtc/api/video_codecs/builtin_video_encoder_factory.h"
#include "webrtc/media/base/media_channel.h"
#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/ssl_adapter.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/field_trial.h"
#if defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/win/msdkvideodecoderfactory.h"
#include "talk/owt/sdk/base/win/msdkvideoencoderfactory.h"
#elif defined(WEBRTC_IOS)
#include "talk/owt/sdk/base/ios/networkmonitorios.h"
#include "talk/owt/sdk/base/objc/ObjcVideoCodecFactory.h"
#endif
#if defined(WEBRTC_LINUX) || defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/customizedvideodecoderfactory.h"
#endif
#include "owt/base/clientconfiguration.h"
#include "owt/base/globalconfiguration.h"
using namespace rtc;
namespace owt {
namespace base {
void PeerConnectionThread::Run() {
  ProcessMessages(kForever);
}
PeerConnectionThread::~PeerConnectionThread() {
  RTC_LOG(LS_INFO) << "Quit a PeerConnectionThread.";
  Stop();
}
rtc::scoped_refptr<PeerConnectionDependencyFactory>
    PeerConnectionDependencyFactory::dependency_factory_;
std::once_flag get_pcdf_once;
PeerConnectionDependencyFactory::PeerConnectionDependencyFactory()
    : pc_thread_(rtc::Thread::CreateWithSocketServer()),
      callback_thread_(rtc::Thread::CreateWithSocketServer()),
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
  pc_thread_->SetName("peerconnection_dependency_factory_thread", nullptr);
  pc_thread_->Start();
}
PeerConnectionDependencyFactory::~PeerConnectionDependencyFactory() {}
rtc::scoped_refptr<webrtc::PeerConnectionInterface>
PeerConnectionDependencyFactory::CreatePeerConnection(
    const webrtc::PeerConnectionInterface::RTCConfiguration& config,
    webrtc::PeerConnectionObserver* observer) {
  return pc_thread_
      ->Invoke<scoped_refptr<webrtc::PeerConnectionInterface>>(
          RTC_FROM_HERE, Bind(&PeerConnectionDependencyFactory::
                                  CreatePeerConnectionOnCurrentThread,
                              this, config, observer))
      .get();
}
PeerConnectionDependencyFactory* PeerConnectionDependencyFactory::Get() {
  std::call_once(get_pcdf_once, []() {
    dependency_factory_ =
        new rtc::RefCountedObject<PeerConnectionDependencyFactory>();
    dependency_factory_->CreatePeerConnectionFactory();
  });
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
  RTC_LOG(LS_INFO) << "CreatePeerConnectionOnCurrentThread";
  if (GlobalConfiguration::GetAECEnabled() &&
      GlobalConfiguration::GetAEC3Enabled()) {
    field_trial_ += "OWT-EchoCanceller3/Enabled/";
  }
  // Set H.264 temporal layers. Ideally it should be set via RtpSenderParam
  int h264_temporal_layers = GlobalConfiguration::GetH264TemporalLayers();
  field_trial_ +=
      "OWT-H264TemporalLayers/" + std::to_string(h264_temporal_layers) + std::string("/");
  webrtc::field_trial::InitFieldTrialsFromString(field_trial_.c_str());
  if (!rtc::InitializeSSL()) {
    RTC_LOG(LS_ERROR) << "Failed to initialize SSL.";
    RTC_NOTREACHED();
    return;
  }
  worker_thread = rtc::Thread::CreateWithSocketServer();

  worker_thread->SetName("worker_thread", nullptr);
  signaling_thread = rtc::Thread::CreateWithSocketServer();

  signaling_thread->SetName("signaling_thread", nullptr);
  network_thread = rtc::Thread::CreateWithSocketServer();

  network_thread->SetName("network_thread", nullptr);
  RTC_CHECK(worker_thread->Start() && signaling_thread->Start() &&
            network_thread->Start())
      << "Failed to start threads";

  // Use webrtc::VideoEn(De)coderFactory on iOS.
  std::unique_ptr<webrtc::VideoEncoderFactory> encoder_factory;
  std::unique_ptr<webrtc::VideoDecoderFactory> decoder_factory;
#if defined(WEBRTC_IOS)
  encoder_factory = ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory();
  decoder_factory = ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory();
#elif defined(WEBRTC_WIN)
  // Configure codec factories. MSDK factory will internally use built-in codecs
  // if hardware acceleration is not in place. For H.265/H.264, if hardware acceleration
  // is turned off at application level, negotiation will fail.
  if (encoded_frame_) {
    encoder_factory.reset(new EncodedVideoEncoderFactory());
  } else if (render_hardware_acceleration_enabled_) {
    encoder_factory.reset(new MSDKVideoEncoderFactory());
  } else {
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
  }

  if (GlobalConfiguration::GetCustomizedVideoDecoderEnabled()) {
    decoder_factory.reset(new CustomizedVideoDecoderFactory(
        GlobalConfiguration::GetCustomizedVideoDecoder()));
  } else if (render_hardware_acceleration_enabled_) {
    decoder_factory.reset(new MSDKVideoDecoderFactory());
  } else {
    decoder_factory = webrtc::CreateBuiltinVideoDecoderFactory();
  }

#elif defined(WEBRTC_LINUX)
  // MSDK support for Linux is not in place. Use default.
  if (encoded_frame_) {
    encoder_factory.reset(new EncodedVideoEncoderFactory());
  } else {
    encoder_factory = std::move(webrtc::CreateBuiltinVideoEncoderFactory());
  }

  if (GlobalConfiguration::GetCustomizedVideoDecoderEnabled()) {
    decoder_factory.reset(new CustomizedVideoDecoderFactory(
        GlobalConfiguration::GetCustomizedVideoDecoder()));
  } else {
    decoder_factory = std::move(webrtc::CreateBuiltinVideoDecoderFactory());
  }
#else
#error "Unsupported platform."
#endif
  rtc::scoped_refptr<AudioDeviceModule> adm;

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  // Raw audio frame
  // if adm is nullptr, voe_base will initilize it with the default internal
  // adm.
  if (GlobalConfiguration::GetCustomizedAudioInputEnabled()) {
    // Create ADM on worker thred as RegisterAudioCallback is invoked there.
    adm = worker_thread->Invoke<rtc::scoped_refptr<AudioDeviceModule>>(
        RTC_FROM_HERE,
        Bind(&PeerConnectionDependencyFactory::
                 CreateCustomizedAudioDeviceModuleOnCurrentThread,
             this));
  }
#endif

  pc_factory_ = webrtc::CreatePeerConnectionFactory(
      network_thread.get(), worker_thread.get(), signaling_thread.get(), adm,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(), std::move(encoder_factory),
      std::move(decoder_factory), nullptr, nullptr);
  pc_factory_->AddRef();
  RTC_LOG(LS_INFO) << "CreatePeerConnectionOnCurrentThread finished.";
}

scoped_refptr<webrtc::PeerConnectionInterface>
PeerConnectionDependencyFactory::CreatePeerConnectionOnCurrentThread(
    const webrtc::PeerConnectionInterface::RTCConfiguration& config,
    webrtc::PeerConnectionObserver* observer) {
  return (pc_factory_->CreatePeerConnection(config, nullptr, nullptr, observer))
      .get();
}
void PeerConnectionDependencyFactory::CreatePeerConnectionFactory() {
  RTC_CHECK(!pc_factory_.get());
  RTC_LOG(LS_INFO)
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
scoped_refptr<VideoTrackInterface>
PeerConnectionDependencyFactory::CreateLocalVideoTrack(
    const std::string& id,
    webrtc::VideoTrackSourceInterface* video_source) {
  return pc_thread_
      ->Invoke<scoped_refptr<VideoTrackInterface>>(
          RTC_FROM_HERE, Bind(&PeerConnectionFactoryInterface::CreateVideoTrack,
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
    options.echo_cancellation =
        absl::optional<bool>(aec_enabled ? true : false);
    options.auto_gain_control =
        absl::optional<bool>(agc_enabled ? true : false);
    options.noise_suppression = absl::optional<bool>(ns_enabled ? true : false);
    options.residual_echo_detector =
        absl::optional<bool>(aec_enabled ? true : false);
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
rtc::NetworkMonitorInterface*
PeerConnectionDependencyFactory::NetworkMonitor() {
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

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
scoped_refptr<webrtc::AudioDeviceModule> PeerConnectionDependencyFactory::
    CreateCustomizedAudioDeviceModuleOnCurrentThread() {
  return CustomizedAudioDeviceModule::Create(
      GlobalConfiguration::GetAudioFrameGenerator());
}
#endif

}  // namespace base
}  // namespace owt
