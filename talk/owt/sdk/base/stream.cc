// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
//
#include "modules/video_capture/video_capture.h"
#include "pc/video_track_source.h"
#include "talk/owt/sdk/base/vcmcapturer.h"
#include "webrtc/api/video/video_source_interface.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/rtc_base/helpers.h"
#include "webrtc/sdk/media_constraints.h"

#include "talk/owt/sdk/base/customizedframescapturer.h"
#include "talk/owt/sdk/base/webrtcaudiorendererimpl.h"
#include "talk/owt/sdk/base/webrtcvideorendererimpl.h"
#include "talk/owt/sdk/include/cpp/owt/base/framegeneratorinterface.h"

#if defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/desktopcapturer.h"
#include "talk/owt/sdk/base/win/videorendererd3d11.h"
#include "webrtc/modules/desktop_capture/desktop_capture_options.h"
#endif
#if defined(WEBRTC_IOS)
#include "talk/owt/sdk/base/objc/ObjcVideoCapturerInterface.h"
#endif
#include "talk/owt/sdk/base/customizedvideosource.h"
#include "talk/owt/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/owt/sdk/base/webrtcvideorendererimpl.h"
#if defined(WEBRTC_WIN)
#include "talk/owt/sdk/base/win/videorendererwin.h"
#endif
#if defined(WEBRTC_LINUX)
#include "talk/owt/sdk/base/linux/videorenderlinux.h"
#endif
#include "talk/owt/sdk/include/cpp/owt/base/deviceutils.h"
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"

using namespace rtc;
namespace owt {
namespace base {

class CapturerTrackSource : public webrtc::VideoTrackSource {
 public:
  static rtc::scoped_refptr<CapturerTrackSource> Create(
      const size_t width,
      const size_t height,
      const size_t fps,
      int capture_device_idx) {
    std::unique_ptr<owt::base::VcmCapturer> capturer;
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (!info) {
      return nullptr;
    }
    int num_devices = info->NumberOfDevices();
    for (int i = 0; i < num_devices; ++i) {
      capturer = absl::WrapUnique(owt::base::VcmCapturer::Create(
          width, height, fps, capture_device_idx));
      if (capturer) {
        return new rtc::RefCountedObject<CapturerTrackSource>(
            std::move(capturer));
      }
    }

    return nullptr;
  }

 protected:
  explicit CapturerTrackSource(std::unique_ptr<owt::base::VcmCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<owt::base::VcmCapturer> capturer_;
};

#if defined(WEBRTC_WIN)
Stream::Stream()
    : media_stream_(nullptr),
      renderer_impl_(nullptr),
      audio_renderer_impl_(nullptr),
      d3d11_renderer_impl_(nullptr),
#ifdef OWT_ENABLE_QUIC
      source_(AudioSourceInfo::kUnknown,
              VideoSourceInfo::kUnknown,
              DataSourceInfo::kUnknown),
#else
      source_(AudioSourceInfo::kUnknown, VideoSourceInfo::kUnknown),
#endif
      ended_(false),
      id_("") {}
Stream::Stream(const std::string& id)
    : media_stream_(nullptr),
      renderer_impl_(nullptr),
      audio_renderer_impl_(nullptr),
      d3d11_renderer_impl_(nullptr),
#ifdef OWT_ENABLE_QUIC
      source_(AudioSourceInfo::kUnknown,
              VideoSourceInfo::kUnknown,
              DataSourceInfo::kUnknown),
#else
      source_(AudioSourceInfo::kUnknown, VideoSourceInfo::kUnknown),
#endif
      ended_(false),
      id_(id) {}
#elif defined(WEBRTC_LINUX)
Stream::Stream()
    : media_stream_(nullptr), renderer_impl_(nullptr), audio_renderer_impl_(nullptr), va_renderer_impl_(nullptr), ended_(false), id_("") {}
Stream::Stream(MediaStreamInterface* media_stream, StreamSourceInfo source)
    : media_stream_(nullptr),   va_renderer_impl_(nullptr),source_(source) {
  MediaStream(media_stream);
}
Stream::Stream(const std::string& id)
    : media_stream_(nullptr), renderer_impl_(nullptr),  audio_renderer_impl_(nullptr), va_renderer_impl_(nullptr), ended_(false), id_(id) {}
#else
Stream::Stream()
    : media_stream_(nullptr), renderer_impl_(nullptr), audio_renderer_impl_(nullptr), ended_(false), id_("") {}
Stream::Stream(MediaStreamInterface* media_stream, StreamSourceInfo source)
    : media_stream_(nullptr), source_(source) {
  MediaStream(media_stream);
}
Stream::Stream(const std::string& id)
    : media_stream_(nullptr), renderer_impl_(nullptr),  audio_renderer_impl_(nullptr), ended_(false), id_(id) {}
#endif

MediaStreamInterface* Stream::MediaStream() const {
  return media_stream_;
}
Stream::~Stream() {
  DetachVideoRenderer();
  DetachAudioPlayer();
  if (media_stream_)
    media_stream_->Release();
}
void Stream::MediaStream(MediaStreamInterface* media_stream) {
  if (media_stream == nullptr) {
    RTC_DCHECK(false);
    return;
  }
  if (media_stream_ != nullptr) {
    media_stream_->Release();
  }
  media_stream_ = media_stream;
  media_stream_->AddRef();
}
std::string Stream::Id() const {
  return id_;
}
void Stream::Id(const std::string& id) {
  id_ = id;
}
void Stream::DisableVideo() {
  SetVideoTracksEnabled(false);
}
void Stream::EnableVideo() {
  SetVideoTracksEnabled(true);
}
void Stream::DisableAudio() {
  SetAudioTracksEnabled(false);
}
void Stream::EnableAudio() {
  SetAudioTracksEnabled(true);
}
void Stream::SetVideoTracksEnabled(bool enabled) {
  if (media_stream_ == nullptr)
    return;
  auto video_tracks = media_stream_->GetVideoTracks();
  for (auto it = video_tracks.begin(); it != video_tracks.end(); ++it) {
    (*it)->set_enabled(enabled);
  }
}
void Stream::SetAudioTracksEnabled(bool enabled) {
  if (media_stream_ == nullptr)
    return;
  auto audio_tracks = media_stream_->GetAudioTracks();
  for (auto it = audio_tracks.begin(); it != audio_tracks.end(); ++it) {
    (*it)->set_enabled(enabled);
  }
}

void Stream::AttachAudioPlayer(AudioPlayerInterface& player) {
  if (media_stream_ == nullptr) {
    RTC_LOG(LS_ERROR) << "Cannot attach to empty stream.";
    return;
  }
  auto audio_tracks = media_stream_->GetAudioTracks();
  if (audio_tracks.size() == 0) {
    RTC_LOG(LS_ERROR) << "Attach failed because of no audio tracks.";
    return;
  } else if (audio_tracks.size() > 1) {
    RTC_LOG(LS_WARNING)
        << "There are more than one audio tracks, the first one "
           "will be attachecd to renderer.";
  }
  WebrtcAudioRendererImpl* old_renderer =
      audio_renderer_impl_ ? audio_renderer_impl_ : nullptr;
  audio_renderer_impl_ = new WebrtcAudioRendererImpl(player);
  audio_tracks[0]->RemoveSink(old_renderer);
  audio_tracks[0]->AddSink(audio_renderer_impl_);
  if (old_renderer)
    delete old_renderer;
  RTC_LOG(LS_INFO) << "Attached the stream to a renderer.";
}

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
void Stream::AttachVideoRenderer(VideoRendererInterface& renderer) {
  if (media_stream_ == nullptr) {
    RTC_LOG(LS_ERROR) << "Cannot attach an audio only stream to a renderer.";
    return;
  }
  auto video_tracks = media_stream_->GetVideoTracks();
  if (video_tracks.size() == 0) {
    RTC_LOG(LS_ERROR) << "Attach failed because of no video tracks.";
    return;
  } else if (video_tracks.size() > 1) {
    RTC_LOG(LS_WARNING)
        << "There are more than one video tracks, the first one "
           "will be attachecd to renderer.";
  }
  WebrtcVideoRendererImpl* old_renderer =
      renderer_impl_ ? renderer_impl_ : nullptr;
  renderer_impl_ = new WebrtcVideoRendererImpl(renderer);
  video_tracks[0]->AddOrUpdateSink(renderer_impl_, rtc::VideoSinkWants());
  if (old_renderer)
    delete old_renderer;
  RTC_LOG(LS_INFO) << "Attached the stream to a renderer.";
}
#endif

#if defined(WEBRTC_LINUX)
void Stream::AttachVideoRenderer(VideoRendererVaInterface& renderer) {
  if (media_stream_ == nullptr) {
    RTC_LOG(LS_ERROR) << "Cannot attach an audio only stream to a renderer.";
    return;
  }
  auto video_tracks = media_stream_->GetVideoTracks();
  if (video_tracks.size() == 0) {
    RTC_LOG(LS_ERROR) << "Attach failed because of no video tracks.";
    return;
  } else if (video_tracks.size() > 1) {
    RTC_LOG(LS_WARNING)
        << "There are more than one video tracks, the first one "
           "will be attachecd to renderer.";
  }

  WebrtcVideoRendererVaImpl* old_renderer =
      va_renderer_impl_ ? va_renderer_impl_ : nullptr;
  va_renderer_impl_ = new WebrtcVideoRendererVaImpl(renderer);
  video_tracks[0]->AddOrUpdateSink(va_renderer_impl_, rtc::VideoSinkWants());
  if (old_renderer)
    delete old_renderer;
  RTC_LOG(LS_INFO) << "Attached the stream to a renderer.";
}

#endif

#if defined(WEBRTC_WIN)
void Stream::AttachVideoRenderer(VideoRenderWindow& render_window) {
  if (media_stream_ == nullptr) {
    RTC_LOG(LS_ERROR) << "Cannot attach an audio only stream to a renderer.";
    return;
  }
  auto video_tracks = media_stream_->GetVideoTracks();
  if (video_tracks.size() == 0) {
    RTC_LOG(LS_ERROR) << "Attach failed because of no video tracks.";
    return;
  } else if (video_tracks.size() > 1) {
    RTC_LOG(LS_WARNING)
        << "There are more than one video tracks, the first one "
           "will be attachecd to renderer.";
  }
  WebrtcVideoRendererD3D11Impl* old_renderer =
      d3d11_renderer_impl_ ? d3d11_renderer_impl_ : nullptr;
  d3d11_renderer_impl_ =
      new WebrtcVideoRendererD3D11Impl(render_window.GetWindowHandle());
  video_tracks[0]->AddOrUpdateSink(d3d11_renderer_impl_, rtc::VideoSinkWants());
  if (old_renderer)
    delete old_renderer;
  RTC_LOG(LS_INFO) << "Attached the stream to a renderer.";
}
#endif

void Stream::DetachVideoRenderer() {
#if defined(WEBRTC_WIN)
  if (media_stream_ == nullptr ||
      (renderer_impl_ == nullptr && d3d11_renderer_impl_ == nullptr))
    return;
#elif defined(WEBRTC_LINUX)
  if (media_stream_ == nullptr ||
      (renderer_impl_ == nullptr && va_renderer_impl_ == nullptr))
    return;
#else
  if (media_stream_ == nullptr || renderer_impl_ == nullptr)
    return;
#endif
  auto video_tracks = media_stream_->GetVideoTracks();
  if (video_tracks.size() == 0)
    return;
  // Detach from the first stream.
  if (renderer_impl_ != nullptr) {
    video_tracks[0]->RemoveSink(renderer_impl_);
    delete renderer_impl_;
    renderer_impl_ = nullptr;
  }
#if defined(WEBRTC_WIN)
  if (d3d11_renderer_impl_ != nullptr) {
    video_tracks[0]->RemoveSink(d3d11_renderer_impl_);
    delete d3d11_renderer_impl_;
    d3d11_renderer_impl_ = nullptr;
  }
#endif
#if defined(WEBRTC_LINUX)
  if (va_renderer_impl_ != nullptr) {
    video_tracks[0]->RemoveSink(va_renderer_impl_);
    delete va_renderer_impl_;
    va_renderer_impl_ = nullptr;
  }
#endif
}

void Stream::DetachAudioPlayer() {
  if (media_stream_ == nullptr)
    return;

  auto audio_tracks = media_stream_->GetAudioTracks();
  if (audio_tracks.size() == 0)
    return;

  if (audio_renderer_impl_ != nullptr) {
    audio_tracks[0]->RemoveSink(audio_renderer_impl_);
    delete audio_renderer_impl_;
    audio_renderer_impl_ = nullptr;
  }
}

StreamSourceInfo Stream::Source() const {
  return source_;
}
void Stream::AddObserver(StreamObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  std::vector<std::reference_wrapper<StreamObserver>>::iterator it =
      std::find_if(observers_.begin(), observers_.end(),
                   [&](std::reference_wrapper<StreamObserver> o) -> bool {
                     return &observer == &(o.get());
                   });
  if (it != observers_.end()) {
    RTC_LOG(LS_INFO) << "Adding duplicate observer.";
    return;
  }
  observers_.push_back(observer);
}
void Stream::RemoveObserver(StreamObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  auto it = std::find_if(observers_.begin(), observers_.end(),
                         [&](std::reference_wrapper<StreamObserver> o) -> bool {
                           return &observer == &(o.get());
                         });
  if (it != observers_.end())
    observers_.erase(it);
}
void Stream::TriggerOnStreamEnded() {
  ended_ = true;
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnEnded();
  }
}
void Stream::TriggerOnStreamUpdated() {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnUpdated();
  }
}
void Stream::TriggerOnStreamMute(TrackKind track_kind) {
  ended_ = true;
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnMute(track_kind);
  }
}
void Stream::TriggerOnStreamUnmute(TrackKind track_kind) {
  ended_ = true;
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnUnmute(track_kind);
  }
}
#if !defined(WEBRTC_WIN)
LocalStream::LocalStream() {}
#if !defined(WEBRTC_LINUX)
LocalStream::LocalStream(MediaStreamInterface* media_stream,
                         StreamSourceInfo source)
    : Stream(media_stream, source) {}
#endif
#endif
LocalStream::~LocalStream() {
  RTC_LOG(LS_INFO) << "Destroy LocalCameraStream.";
  if (media_stream_ != nullptr) {
    // Remove all tracks before dispose stream.
    auto audio_tracks = media_stream_->GetAudioTracks();
    for (auto it = audio_tracks.begin(); it != audio_tracks.end(); ++it) {
      media_stream_->RemoveTrack(*it);
    }
    auto video_tracks = media_stream_->GetVideoTracks();
    for (auto it = video_tracks.begin(); it != video_tracks.end(); ++it) {
      media_stream_->RemoveTrack(*it);
    }
  }
}

std::shared_ptr<LocalStream> LocalStream::Create(
    const LocalCameraStreamParameters& parameters,
    int& error_code) {
  error_code = 0;
  std::shared_ptr<LocalStream> stream(new LocalStream(parameters, error_code));
  if (error_code != 0)
    return nullptr;
  else
    return stream;
}
std::shared_ptr<LocalStream> LocalStream::Create(
    const bool is_audio_enabled,
    webrtc::VideoTrackSourceInterface* video_source,
    int& error_code) {
  error_code = 0;
  std::shared_ptr<LocalStream> stream(
      new LocalStream(is_audio_enabled, video_source, error_code));
  if (error_code != 0)
    return nullptr;
  else
    return stream;
}
#if defined(WEBRTC_WIN)
std::shared_ptr<LocalStream> LocalStream::Create(
    std::shared_ptr<LocalDesktopStreamParameters> parameters,
    std::unique_ptr<LocalScreenStreamObserver> observer) {
  std::shared_ptr<LocalStream> stream(
      new LocalStream(parameters, std::move(observer)));
  return stream;
}
#endif
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
std::shared_ptr<LocalStream> LocalStream::Create(
    std::shared_ptr<LocalCustomizedStreamParameters> parameters,
    std::unique_ptr<VideoFrameGeneratorInterface> framer) {
  std::shared_ptr<LocalStream> stream(
      new LocalStream(parameters, std::move(framer)));
  return stream;
}
std::shared_ptr<LocalStream> LocalStream::Create(
    std::shared_ptr<LocalCustomizedStreamParameters> parameters,
    VideoEncoderInterface* encoder) {
  std::shared_ptr<LocalStream> stream(new LocalStream(parameters, encoder));
  return stream;
}
#endif

#ifdef OWT_ENABLE_QUIC
LocalStream::LocalStream(std::shared_ptr<QuicStream> quic_stream) {
  quic_stream_ = quic_stream;
  has_data_ = true;
}

std::shared_ptr<LocalStream> LocalStream::Create(
    std::shared_ptr<QuicStream> writable_stream,
    int& error_code) {
  std::shared_ptr<LocalStream> stream(new LocalStream(writable_stream));
  error_code = 0;
  return stream;
}
#endif

LocalStream::LocalStream(const LocalCameraStreamParameters& parameters,
                         int& error_code) {
  if (!parameters.AudioEnabled() && !parameters.VideoEnabled()) {
    RTC_LOG(LS_ERROR)
        << "Cannot create a LocalCameraStream without audio and video.";
    error_code = static_cast<int>(ExceptionType::kLocalInvalidOption);
    return;
  }
  scoped_refptr<PeerConnectionDependencyFactory> pcd_factory =
      PeerConnectionDependencyFactory::Get();
  std::string media_stream_id("MediaStream-" + rtc::CreateRandomUuid());
  Id(media_stream_id);
  scoped_refptr<MediaStreamInterface> stream =
      pcd_factory->CreateLocalMediaStream(media_stream_id);
  // Create audio track
  if (parameters.AudioEnabled()) {
    std::string audio_track_id(rtc::CreateRandomUuid());
    scoped_refptr<AudioTrackInterface> audio_track =
        pcd_factory->CreateLocalAudioTrack("AudioTrack-" + audio_track_id);
    stream->AddTrack(audio_track);
  }
  // Create video track.
  if (parameters.VideoEnabled()) {
#if !defined(WEBRTC_IOS)
    rtc::scoped_refptr<webrtc::VideoCaptureModule> capturer(nullptr);

    // Check supported resolution
    auto supported_resolution =
        DeviceUtils::VideoCapturerSupportedResolutions(parameters.CameraId());
    auto resolution_iterator =
        std::find(supported_resolution.begin(), supported_resolution.end(),
                  Resolution(parameters.ResolutionWidth(),
                             parameters.ResolutionHeight()));
    if (resolution_iterator == supported_resolution.end()) {
      RTC_LOG(LS_ERROR) << "Resolution " << parameters.ResolutionWidth() << "x"
                        << parameters.ResolutionHeight()
                        << " is not supported by video capturer "
                        << parameters.CameraId();
      error_code = static_cast<int>(ExceptionType::kLocalNotSupported);
      return;
    }
    rtc::scoped_refptr<CapturerTrackSource> source =
        CapturerTrackSource::Create(
            parameters.ResolutionWidth(), parameters.ResolutionHeight(),
            parameters.Fps(),
            DeviceUtils::GetVideoCaptureDeviceIndex(parameters.CameraId()));
#else
    capturer_ = ObjcVideoCapturerFactory::Create(parameters);
    if (!capturer_) {
      RTC_LOG(LS_ERROR)
          << "Failed to create capturer. Please check parameters.";
      error_code = static_cast<int>(ExceptionType::kLocalNotSupported);
      return;
    }
    scoped_refptr<VideoTrackSourceInterface> source = capturer_->source();
#endif
    if (!source) {
      RTC_LOG(LS_ERROR)
          << "Failed to create video source. Please check device availability.";
      error_code = static_cast<int>(ExceptionType::kLocalNotSupported);
      return;
    }
    std::string video_track_id("VideoTrack-" + rtc::CreateRandomUuid());
    scoped_refptr<VideoTrackInterface> video_track =
        pcd_factory->CreateLocalVideoTrack(video_track_id, source);
    if (!video_track) {
      RTC_LOG(LS_ERROR)
          << "Failed to create video track. Please check device availability.";
      error_code = static_cast<int>(ExceptionType::kLocalNotSupported);
      return;
    }
    stream->AddTrack(video_track);
  }
  source_.video = VideoSourceInfo::kCamera;
  source_.audio = AudioSourceInfo::kMic;
  media_stream_ = stream;
  media_stream_->AddRef();
}

LocalStream::LocalStream(const bool is_audio_enabled,
                         webrtc::VideoTrackSourceInterface* video_source,
                         int& error_code) {
  RTC_CHECK(video_source);
  scoped_refptr<PeerConnectionDependencyFactory> pcd_factory =
      PeerConnectionDependencyFactory::Get();
  std::string media_stream_id("MediaStream-" + rtc::CreateRandomUuid());
  Id(media_stream_id);
  scoped_refptr<MediaStreamInterface> stream =
      pcd_factory->CreateLocalMediaStream(media_stream_id);
  // Create audio track
  if (is_audio_enabled) {
    std::string audio_track_id(rtc::CreateRandomUuid());
    scoped_refptr<AudioTrackInterface> audio_track =
        pcd_factory->CreateLocalAudioTrack("AudioTrack-" + audio_track_id);
    stream->AddTrack(audio_track);
  }
  std::string video_track_id("VideoTrack-" + rtc::CreateRandomUuid());
  scoped_refptr<VideoTrackInterface> video_track =
      pcd_factory->CreateLocalVideoTrack(video_track_id, video_source);
  stream->AddTrack(video_track);
  media_stream_ = stream;
  media_stream_->AddRef();
}
void LocalStream::Close() {
  RTC_CHECK(media_stream_);
  DetachVideoRenderer();
  for (auto const& audio_track : media_stream_->GetAudioTracks())
    media_stream_->RemoveTrack(audio_track);
  for (auto const& video_track : media_stream_->GetVideoTracks())
    media_stream_->RemoveTrack(video_track);
}
#if defined(WEBRTC_WIN)
LocalStream::LocalStream(
    std::shared_ptr<LocalDesktopStreamParameters> parameters,
    std::unique_ptr<LocalScreenStreamObserver> observer) {
  if (!parameters->VideoEnabled() && !parameters->AudioEnabled()) {
    RTC_LOG(LS_WARNING) << "Create LocalScreenStream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> factory =
      PeerConnectionDependencyFactory::Get();
  std::string media_stream_id("MediaStream-" + rtc::CreateRandomUuid());
  Id(media_stream_id);
  scoped_refptr<MediaStreamInterface> stream =
      factory->CreateLocalMediaStream(media_stream_id);
  std::unique_ptr<BasicDesktopCapturer> capturer(nullptr);
  if (parameters->VideoEnabled()) {
    rtc::scoped_refptr<LocalDesktopCaptureTrackSource> video_device =
        LocalDesktopCaptureTrackSource::Create(parameters, std::move(observer));
    if (video_device) {
      std::string video_track_id("VideoTrack-" + rtc::CreateRandomUuid());
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track =
          factory->CreateLocalVideoTrack(video_track_id, video_device);
      stream->AddTrack(video_track);
    }
  }
  if (parameters->AudioEnabled()) {
    std::string audio_track_id("AudioTrack-" + rtc::CreateRandomUuid());
    scoped_refptr<AudioTrackInterface> audio_track =
        factory->CreateLocalAudioTrack(audio_track_id);
    stream->AddTrack(audio_track);
  }
  media_stream_ = stream;
  media_stream_->AddRef();
  source_.video = VideoSourceInfo::kScreenCast;
}
#endif

#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
LocalStream::LocalStream(
    std::shared_ptr<LocalCustomizedStreamParameters> parameters,
    std::unique_ptr<VideoFrameGeneratorInterface> framer) {
  if (!parameters->VideoEnabled() && !parameters->AudioEnabled()) {
    RTC_LOG(LS_WARNING)
        << "Create Local Camera Stream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> pcd_factory =
      PeerConnectionDependencyFactory::Get();
  std::string media_stream_id("MediaStream-" + rtc::CreateRandomUuid());
  Id(media_stream_id);
  scoped_refptr<MediaStreamInterface> stream =
      pcd_factory->CreateLocalMediaStream(media_stream_id);
  std::unique_ptr<CustomizedFramesCapturer> capturer(nullptr);
  if (parameters->VideoEnabled()) {
    rtc::scoped_refptr<LocalRawCaptureTrackSource> video_device =
        LocalRawCaptureTrackSource::Create(parameters, std::move(framer));
    if (video_device) {
      std::string video_track_id("VideoTrack-" + rtc::CreateRandomUuid());
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track =
          pcd_factory->CreateLocalVideoTrack(video_track_id, video_device);
      stream->AddTrack(video_track);
    }
  }
  if (parameters->AudioEnabled()) {
    std::string audio_track_id("AudioTrack-" + rtc::CreateRandomUuid());
    scoped_refptr<AudioTrackInterface> audio_track =
        pcd_factory->CreateLocalAudioTrack(audio_track_id);
    stream->AddTrack(audio_track);
  }
  media_stream_ = stream;
  media_stream_->AddRef();
}
LocalStream::LocalStream(
    std::shared_ptr<LocalCustomizedStreamParameters> parameters,
    VideoEncoderInterface* encoder) {
  if (!parameters->VideoEnabled() && !parameters->AudioEnabled()) {
    RTC_LOG(LS_WARNING) << "Create LocalStream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> pcd_factory =
      PeerConnectionDependencyFactory::Get();
  std::string media_stream_id("MediaStream-" + rtc::CreateRandomUuid());
  Id(media_stream_id);
  scoped_refptr<MediaStreamInterface> stream =
      pcd_factory->CreateLocalMediaStream(media_stream_id);
  std::unique_ptr<CustomizedFramesCapturer> capturer(nullptr);
  if (parameters->VideoEnabled()) {
    encoded_ = true;
    rtc::scoped_refptr<LocalEncodedCaptureTrackSource> video_device =
        LocalEncodedCaptureTrackSource::Create(parameters, encoder);
    if (video_device) {
      std::string video_track_id("VideoTrack-" + rtc::CreateRandomUuid());
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track =
          pcd_factory->CreateLocalVideoTrack(video_track_id, video_device);
      stream->AddTrack(video_track);
    }
  }
  if (parameters->AudioEnabled()) {
    std::string audio_track_id("AudioTrack-" + rtc::CreateRandomUuid());
    scoped_refptr<AudioTrackInterface> audio_track =
        pcd_factory->CreateLocalAudioTrack(audio_track_id);
    stream->AddTrack(audio_track);
  }
  media_stream_ = stream;
  media_stream_->AddRef();
}
#endif

RemoteStream::RemoteStream(MediaStreamInterface* media_stream,
                           const std::string& from)
    : origin_(from) {
  RTC_CHECK(media_stream);
  Id(media_stream->id());
  media_stream_ = media_stream;
  media_stream_->AddRef();
}

RemoteStream::RemoteStream(
    const std::string& id,
    const std::string& from,
    const owt::base::SubscriptionCapabilities& subscription_capabilities,
    const owt::base::PublicationSettings& publication_settings)
    : Stream(id),
      origin_(from),
      subscription_capabilities_(subscription_capabilities),
      publication_settings_(publication_settings) {}

std::string RemoteStream::Origin() {
  return origin_;
}

RemoteStream::RemoteStream(const std::string& id, const std::string& from)
    :Stream(id),
     origin_(from) {
}

void RemoteStream::MediaStream(MediaStreamInterface* media_stream) {
  Stream::MediaStream(media_stream);
}
MediaStreamInterface* RemoteStream::MediaStream() {
  return media_stream_;
}

#ifdef OWT_ENABLE_QUIC
QuicStream::QuicStream(owt::quic::QuicTransportStreamInterface* quic_stream,
           const std::string& session_id)
    : quic_stream_(quic_stream), session_id_(session_id), can_read_(true),
      can_write_(true), fin_read_(false) {
}

QuicStream::~QuicStream() {
  if (quic_stream_) {
    delete quic_stream_;
    quic_stream_ = nullptr;
  }
}

size_t QuicStream::Write(uint8_t* data, size_t length) {
  if (quic_stream_ && !fin_read_ && data != nullptr && length > 0) {
    return quic_stream_->Write(data, length);
  }
  return 0;
}

size_t QuicStream::Read(uint8_t* data, size_t length) {
  if (quic_stream_ && data != nullptr && length > 0 && !fin_read_) {
    return quic_stream_->Read(data, length);
  } else {
    return 0;
  }
}

size_t QuicStream::ReadableBytes() const {
  if (quic_stream_ && !fin_read_) {
    return quic_stream_->ReadableBytes();
  } else {
    return 0;
  }
}

std::shared_ptr<owt::base::QuicStream> LocalStream::Stream() {
  return quic_stream_;
}
#endif
}  // namespace base
}  // namespace owt
