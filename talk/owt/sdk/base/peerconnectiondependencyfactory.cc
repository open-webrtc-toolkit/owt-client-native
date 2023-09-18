// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
//
#include "talk/owt/sdk/base/customizedaudiodevicemodule.h"
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
#include <d3d11.h>
#include "talk/owt/sdk/base/win/externalvideodecoderfactory.h"
#include "talk/owt/sdk/base/win/externalvideoencoderfactory.h"
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
  if (GlobalConfiguration::GetLowLatencyStreamingEnabled()) {
    field_trial_ += "OWT-LowLatencyMode/Enabled/";
  }

  if (GlobalConfiguration::GetAECEnabled() &&
      GlobalConfiguration::GetAEC3Enabled()) {
    field_trial_ += "OWT-EchoCanceller3/Enabled/";
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
  field_trial_ += "OWT-H264TemporalLayers/" +
                  std::to_string(h264_temporal_layers) + std::string("/");
  int link_mtu = GlobalConfiguration::GetLinkMTU();
  if (link_mtu > 0) {
    field_trial_ +=
        "OWT-LinkMTU/" + std::to_string(link_mtu) + std::string("/");
  }
  if (GlobalConfiguration::GetFlexFecEnabled()) {
    field_trial_ += "OWT-FlexFEC/Enabled/";
  }
  if (GlobalConfiguration::GetRangeExtensionEnabled()) {
    field_trial_ += "OWT-RangeExtension/Enabled/";
  }
  int delay_bwe_weight = GlobalConfiguration::GetDelayBasedBweWeight();
  field_trial_ +=
      "OWT-DelayBweWeight/" + std::to_string(delay_bwe_weight) + "/";
  int start_bitrate, min_bitrate, max_bitrate;
  GlobalConfiguration::GetBweRateLimits(start_bitrate, min_bitrate,
                                        max_bitrate);
  if (start_bitrate > 0 || min_bitrate > 0 || max_bitrate > 0)
  field_trial_ += "OWT-Bwe-RateLimits/start:" + std::to_string(start_bitrate) + ",min:" +
                  std::to_string(min_bitrate) + ",max:" +
                  std::to_string(max_bitrate) + "/";
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
  packet_socket_factory_ = std::make_shared<rtc::BasicPacketSocketFactory>(
      network_thread->socketserver());
  network_manager_ = std::make_shared<rtc::BasicNetworkManager>(
      network_thread->socketserver());
  std::unique_ptr<webrtc::VideoEncoderFactory> encoder_factory;
  std::unique_ptr<webrtc::VideoDecoderFactory> decoder_factory;
#if defined(WEBRTC_IOS)
  encoder_factory = ObjcVideoCodecFactory::CreateObjcVideoEncoderFactory();
  decoder_factory = ObjcVideoCodecFactory::CreateObjcVideoDecoderFactory();
#elif defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  // Configure codec factories. MSDK factory will internally use built-in codecs
  // if hardware acceleration is not in place. For H.265/H.264, if hardware
  // acceleration is turned off at application level, negotiation will fail.
  if (encoded_frame_) {
    encoder_factory.reset(new EncodedVideoEncoderFactory());
  } else if (render_hardware_acceleration_enabled_) {
#if defined(OWT_CG_CLIENT)
    // CG client app takes care of external encoder. If it's expected to receive
    // video streams, an encoder must be provided.
    encoder_factory.reset(new ExternalVideoEncoderFactory());
#elif defined(WEBRTC_WIN) && defined(OWT_USE_MSDK)
    encoder_factory.reset(new ExternalVideoEncoderFactory());
#else
    // For Linux HW encoder pending verification.
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
#endif
  } else {
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
  }

  if (GlobalConfiguration::GetCustomizedVideoDecoderEnabled()) {
    decoder_factory.reset(new CustomizedVideoDecoderFactory(
        GlobalConfiguration::GetCustomizedVideoDecoder()));
  } else if (render_hardware_acceleration_enabled_) {
#if defined(OWT_USE_MSDK) || defined(OWT_USE_FFMPEG) || defined(OWT_CG_SERVER)
#if defined(WEBRTC_WIN)
    decoder_factory.reset(new ExternalVideoDecoderFactory(nullptr));
#elif !defined(OWT_CG_SERVER)
    // Linux CG server is supposed to have external decoder so it can receives
    // video streams from client.
    decoder_factory.reset(new ExternalVideoDecoderFactory());
#endif
#else
    decoder_factory = webrtc::CreateBuiltinVideoDecoderFactory();
#endif
  } else {
    decoder_factory = webrtc::CreateBuiltinVideoDecoderFactory();
  }
#endif
  // If still video factory is not in place, use internal factory.
  if (!encoder_factory.get()) {
    encoder_factory = webrtc::CreateBuiltinVideoEncoderFactory();
  }
  if (!decoder_factory.get()) {
    decoder_factory = webrtc::CreateBuiltinVideoDecoderFactory();
  }

  // Raw audio frame
  // if adm is nullptr, voe_base will initilize it with the default internal
  // adm.
  rtc::scoped_refptr<AudioDeviceModule> adm;
  if (GlobalConfiguration::GetCustomizedAudioInputEnabled()) {
    // Create ADM on worker thred as RegisterAudioCallback is invoked there.
    adm = worker_thread->BlockingCall(
        [this] { return CreateCustomizedAudioDeviceModuleOnCurrentThread(); });
  } else {
#if defined(WEBRTC_WIN)
    // For Windows we create the audio device with non audio_device_impl
    // dependent factory to facilitate switching of playback devices.
    task_queue_factory_ = CreateDefaultTaskQueueFactory();
    com_initializer_ = std::make_unique<webrtc::ScopedCOMInitializer>(
        webrtc::ScopedCOMInitializer::kMTA);
    if (com_initializer_->Succeeded()) {
      adm = worker_thread->BlockingCall([this] {
        return CreateWindowsCoreAudioAudioDeviceModule(
            task_queue_factory_.get(), true);
      });
    }
#endif
  }
#if defined(WEBRTC_IOS)
  pc_factory_ = webrtc::CreatePeerConnectionFactory(
      network_thread.get(), worker_thread.get(), signaling_thread.get(), adm,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(), std::move(encoder_factory),
      std::move(decoder_factory), nullptr,
      nullptr);  // Decoder factory
#elif defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  pc_factory_ = webrtc::CreatePeerConnectionFactory(
      network_thread.get(), worker_thread.get(), signaling_thread.get(), adm,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      std::move(encoder_factory),   // Encoder factory
      std::move(decoder_factory), nullptr, nullptr);  // Decoder factory
#else
#error "Unsupported platform."
#endif
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
  int min_port = 0;
  int max_port = 0;
  GlobalConfiguration::GetIcePortAllocationRanges(min_port, max_port);
  if (min_port > 0 && max_port > 0 && max_port >= min_port) {
    port_allocator->SetPortRange(min_port, max_port);
  }
  return pc_factory_->CreatePeerConnection(config, std::move(port_allocator),
                                           nullptr, observer);
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
#ifdef OWT_LOW_LATENCY
    options.highpass_filter = absl::optional<bool>(false);
    options.typing_detection = absl::optional<bool>(false);
    options.experimental_agc = absl::optional<bool>(false);
    options.experimental_ns = absl::optional<bool>(false);
#endif
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

scoped_refptr<webrtc::AudioDeviceModule> PeerConnectionDependencyFactory::
    CreateCustomizedAudioDeviceModuleOnCurrentThread() {
  return CustomizedAudioDeviceModule::Create(
      GlobalConfiguration::GetAudioFrameGenerator());
}

}  // namespace base
}  // namespace owt
