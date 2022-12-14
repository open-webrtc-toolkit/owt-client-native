// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
//
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
#if defined(WEBRTC_WIN)
#include "webrtc/modules/audio_device/include/audio_device_factory.h"
#endif
#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/p2p/client/basic_port_allocator.h"
#include "webrtc/rtc_base/ssl_adapter.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/system_wrappers/include/field_trial.h"
#include "webrtc/rtc_base/physical_socket_server.h"
#if defined(WEBRTC_WIN)
#ifdef OWT_USE_MSDK
#include "talk/owt/sdk/base/win/msdkvideodecoderfactory.h"
#include "talk/owt/sdk/base/win/msdkvideoencoderfactory.h"
#endif
#elif defined(WEBRTC_LINUX)
#include "talk/owt/sdk/base/linux/msdkvideodecoderfactory.h"
#elif defined(WEBRTC_IOS)
#include "talk/owt/sdk/base/objc/ObjcVideoCodecFactory.h"
#endif
#if defined(WEBRTC_LINUX) || defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/customizedvideodecoderfactory.h"
#endif
#include "owt/base/clientconfiguration.h"
#include "owt/base/globalconfiguration.h"

#if defined(WEBRTC_WIN) && defined(_MSC_VER)
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "dmoguids.lib")
#pragma comment(lib, "msdmo.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "amstrmid.lib")
#pragma comment(lib, "strmiids.lib")
#ifdef OWT_USE_MSDK
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "libmfx_vs2015.lib")
#endif
#ifdef OWT_USE_OPENSSL
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#endif
#ifdef OWT_ENABLE_QUIC
#pragma comment(lib, "owt_web_transport.dll.lib")
#endif
#endif

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
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  if (GlobalConfiguration::GetVideoHardwareAccelerationEnabled()) {
    render_hardware_acceleration_enabled_ = true;
  } else {
    render_hardware_acceleration_enabled_ = false;
  }
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
  return pc_thread_->BlockingCall([this, &config, &observer] {
    return CreatePeerConnectionOnCurrentThread(config, observer);
  });
}
PeerConnectionDependencyFactory* PeerConnectionDependencyFactory::Get() {
  std::call_once(get_pcdf_once, []() {
    dependency_factory_ =
        rtc::make_ref_counted<PeerConnectionDependencyFactory>();
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
  // Handle port ranges.
  IcePortRanges ice_port_ranges;
  GlobalConfiguration::GetPortRanges(ice_port_ranges);
  if ((ice_port_ranges.audio.min > 0 &&
       ice_port_ranges.audio.max > ice_port_ranges.audio.min) ||
      (ice_port_ranges.video.min > 0 &&
       ice_port_ranges.video.max > ice_port_ranges.video.min) ||
      (ice_port_ranges.screen.min > 0 &&
       ice_port_ranges.screen.max > ice_port_ranges.screen.min) ||
      (ice_port_ranges.data.min > 0 &&
       ice_port_ranges.data.max > ice_port_ranges.data.min)) {
    field_trial_ += "OWT-IceUnbundle/Enabled/";
  }
  bool pre_decode_dump = GlobalConfiguration::GetPreDecodeDumpEnabled();
  if (pre_decode_dump) {
    field_trial_ += "WebRTC-DecoderDataDumpDirectory/./";
  }

  bool post_encode_dump = GlobalConfiguration::GetPostEncodeDumpEnabled();
  if (post_encode_dump) {
    field_trial_ += "WebRTC-EncoderDataDumpDirectory/./";
  }

  // Set H.264 temporal layers. Ideally it should be set via RtpSenderParam
  int h264_temporal_layers = GlobalConfiguration::GetH264TemporalLayers();
  field_trial_ +=
      "OWT-H264TemporalLayers/" + std::to_string(h264_temporal_layers) + std::string("/");
  webrtc::field_trial::InitFieldTrialsFromString(field_trial_.c_str());
  if (!rtc::InitializeSSL()) {
    RTC_LOG(LS_ERROR) << "Failed to initialize SSL.";
    RTC_DCHECK_NOTREACHED();
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

  network_manager_ = std::make_shared<rtc::BasicNetworkManager>(
      network_thread->socketserver());
  packet_socket_factory_ = std::make_shared<rtc::BasicPacketSocketFactory>(
      network_thread->socketserver());

  // Use webrtc::VideoEn(De)coderFactory on iOS.
  std::unique_ptr<webrtc::VideoEncoderFactory> encoder_factory;
  std::unique_ptr<webrtc::VideoDecoderFactory> decoder_factory;
#if defined(WEBRTC_IOS)
  encoder_factory = ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory();
  decoder_factory = ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory();
#elif defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  // Configure codec factories. MSDK factory will internally use built-in codecs
  // if hardware acceleration is not in place. For H.265/H.264, if hardware acceleration
  // is turned off at application level, negotiation will fail.
  if (encoded_frame_) {
    encoder_factory.reset(new EncodedVideoEncoderFactory());
  } else if (render_hardware_acceleration_enabled_) {
#if defined(WEBRTC_LINUX)
    // For Linux HW encoder pending verification.
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
#else
#ifdef OWT_USE_MSDK
    encoder_factory.reset(new MSDKVideoEncoderFactory());
#else
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
#endif
#endif
  } else {
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
  }

  if (GlobalConfiguration::GetCustomizedVideoDecoderEnabled()) {
    decoder_factory.reset(new CustomizedVideoDecoderFactory(
        GlobalConfiguration::GetCustomizedVideoDecoder()));
  } else if (render_hardware_acceleration_enabled_) {
#ifdef OWT_USE_MSDK
    decoder_factory.reset(new MSDKVideoDecoderFactory());
#else
    decoder_factory = webrtc::CreateBuiltinVideoDecoderFactory();
#endif
  } else {
    decoder_factory = webrtc::CreateBuiltinVideoDecoderFactory();
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
    adm = worker_thread->BlockingCall(
        [this] { return CreateCustomizedAudioDeviceModuleOnCurrentThread(); });
  } else {
#if defined(WEBRTC_WIN)
    // For Widnows we create the audio device with non audio_device_impl
    // dependent factory to facilitate switching of playback devices.
    task_queue_factory_ = CreateDefaultTaskQueueFactory();
    com_initializer_ = std::make_unique<webrtc::ScopedCOMInitializer>(
        webrtc::ScopedCOMInitializer::kMTA);
    if (com_initializer_->Succeeded()) {
      // Ensures that the adm is used on the same thread as it is constructed
      adm = worker_thread->Invoke<rtc::scoped_refptr<AudioDeviceModule>>(
          RTC_FROM_HERE, [&] {
            return CreateWindowsCoreAudioAudioDeviceModule(
                task_queue_factory_.get(), true);
          });
    }
#endif
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
  std::unique_ptr<cricket::PortAllocator> port_allocator;
  port_allocator.reset(new cricket::BasicPortAllocator(
      network_manager_.get(), packet_socket_factory_.get()));
  return (pc_factory_->CreatePeerConnection(config, std::move(port_allocator),
                                            nullptr, observer));
}
void PeerConnectionDependencyFactory::CreatePeerConnectionFactory() {
  RTC_CHECK(!pc_factory_.get());
  RTC_LOG(LS_INFO)
      << "PeerConnectionDependencyFactory::CreatePeerConnectionFactory()";
  RTC_CHECK(pc_thread_);
  pc_thread_->BlockingCall(
      [this]() { CreatePeerConnectionFactoryOnCurrentThread(); });
  RTC_CHECK(pc_factory_.get());
}

scoped_refptr<webrtc::MediaStreamInterface>
PeerConnectionDependencyFactory::CreateLocalMediaStream(
    const std::string& label) {
  RTC_CHECK(pc_thread_);
  return pc_thread_->BlockingCall(
      [this, &label]() { return pc_factory_->CreateLocalMediaStream(label); });
}

scoped_refptr<VideoTrackInterface>
PeerConnectionDependencyFactory::CreateLocalVideoTrack(
    const std::string& id,
    webrtc::VideoTrackSourceInterface* video_source) {
  return pc_thread_->BlockingCall([this, &id, &video_source] {
    return pc_factory_->CreateVideoTrack(id, video_source);
  });
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
    scoped_refptr<webrtc::AudioSourceInterface> audio_source =
        CreateAudioSource(options);
    return pc_thread_->BlockingCall([this, &id, &audio_source] {
      return pc_factory_->CreateAudioTrack(id, audio_source.get());
    });
  } else {
    return pc_thread_->BlockingCall(
        [this, &id] { return pc_factory_->CreateAudioTrack(id, nullptr); });
  }
}

scoped_refptr<AudioTrackInterface>
PeerConnectionDependencyFactory::CreateLocalAudioTrack(
    const std::string& id,
    webrtc::AudioSourceInterface* audio_source) {
  return pc_thread_->BlockingCall([this, &id, &audio_source] {
    return pc_factory_->CreateAudioTrack(id, audio_source);
  });
}

rtc::scoped_refptr<AudioSourceInterface>
PeerConnectionDependencyFactory::CreateAudioSource(
    const cricket::AudioOptions& options) {
  return pc_thread_->BlockingCall(
      [this, &options] { return pc_factory_->CreateAudioSource(options); });
}

rtc::scoped_refptr<PeerConnectionFactoryInterface>
PeerConnectionDependencyFactory::PeerConnectionFactory() const {
  return pc_factory_;
}

rtc::Thread* PeerConnectionDependencyFactory::SignalingThreadForTesting() {
  return signaling_thread.get();
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
