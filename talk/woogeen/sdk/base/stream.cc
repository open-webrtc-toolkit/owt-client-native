/*
 * Intel License
 */

#include <random>

#include "talk/media/base/videocapturer.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/woogeen/sdk/base/stream.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"

namespace woogeen {

Stream::Stream() : id_("") {}

Stream::Stream(std::string& id) : id_(id) {}

scoped_refptr<MediaStreamInterface> Stream::MediaStream() const {
  RTC_CHECK(media_stream_);
  return media_stream_;
}

void Stream::MediaStream(scoped_refptr<MediaStreamInterface> media_stream) {
  RTC_CHECK(media_stream);
  media_stream_ = media_stream;
}

const std::string& Stream::Id() const {
  return id_;
}

void Stream::Id(std::string& id) {
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

LocalCameraStream::~LocalCameraStream() {
  LOG(LS_INFO) << "Destory LocalCameraStream.";
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

LocalCameraStream::LocalCameraStream(
    std::shared_ptr<LocalCameraStreamParameters> parameters) {
  if (!parameters->VideoEnabled() && !parameters->AudioEnabled()) {
    LOG(LS_WARNING) << "Create LocalCameraStream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> factory =
      PeerConnectionDependencyFactory::Get();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 99999999);
  std::string media_stream_label =
      "MediaStream-" + std::to_string(dis(gen));  // TODO: use UUID.
  scoped_refptr<MediaStreamInterface> stream =
      factory->CreateLocalMediaStream(media_stream_label);
  if (parameters->VideoEnabled()) {
    rtc::scoped_ptr<cricket::DeviceManagerInterface> device_manager(
        cricket::DeviceManagerFactory::Create());
    bool initialized = device_manager->Init();
    cricket::Device device;
    if (!device_manager->GetVideoCaptureDevice(parameters->CameraId(),
                                               &device)) {
      LOG(LS_ERROR) << "GetVideoCaptureDevice failed";
      return;
    }
    rtc::scoped_ptr<cricket::VideoCapturer> capturer(
        device_manager->CreateVideoCapturer(device));
    cricket::VideoCapturer* capturer_ptr = capturer.release();
    media_constraints_.SetMandatory(
        webrtc::MediaConstraintsInterface::kMaxWidth,
        std::to_string(parameters->ResolutionWidth()));
    media_constraints_.SetMandatory(
        webrtc::MediaConstraintsInterface::kMaxHeight,
        std::to_string(parameters->ResolutionHeight()));
    media_constraints_.SetMandatory(
        webrtc::MediaConstraintsInterface::kMinWidth,
        std::to_string(parameters->ResolutionWidth()));
    media_constraints_.SetMandatory(
        webrtc::MediaConstraintsInterface::kMinHeight,
        std::to_string(parameters->ResolutionHeight()));
    scoped_refptr<VideoSourceInterface> source =
        factory->CreateVideoSource(capturer_ptr, &media_constraints_);
    std::string video_track_label =
        "VideoTrack-" + std::to_string(dis(gen));  // TODO: use UUID.
    scoped_refptr<VideoTrackInterface> video_track =
        factory->CreateLocalVideoTrack(video_track_label, source);
    stream->AddTrack(video_track);
  }
  if (parameters->AudioEnabled()) {
    std::string audio_track_label =
        "AudioTrack-" + std::to_string(dis(gen));  // TODO: use UUID.
    scoped_refptr<AudioTrackInterface> audio_track =
        factory->CreateLocalAudioTrack(audio_track_label);
    stream->AddTrack(audio_track);
  }
  media_stream_ = stream;
}

RemoteStream::RemoteStream(MediaStreamInterface* media_stream,
                           std::string& from)
    : remote_user_id_(from) {
  media_stream_ = media_stream;
}

RemoteStream::RemoteStream(std::string& id, std::string& from)
    : Stream(id), remote_user_id_(from) {}
std::string& RemoteStream::From() {
  return remote_user_id_;
}

void RemoteStream::MediaStream(
    scoped_refptr<MediaStreamInterface> media_stream) {
  media_stream_ = media_stream;
}

scoped_refptr<MediaStreamInterface> RemoteStream::MediaStream() {
  return media_stream_;
}

RemoteCameraStream::RemoteCameraStream(std::string& id, std::string& from)
    : RemoteStream(id, from) {}

RemoteCameraStream::RemoteCameraStream(MediaStreamInterface* media_stream,
                                       std::string& from)
    : RemoteStream(media_stream, from) {}

RemoteScreenStream::RemoteScreenStream(std::string& id, std::string& from)
    : RemoteStream(id, from) {}

RemoteScreenStream::RemoteScreenStream(MediaStreamInterface* media_stream,
                                       std::string& from)
    : RemoteStream(media_stream, from) {}
}
