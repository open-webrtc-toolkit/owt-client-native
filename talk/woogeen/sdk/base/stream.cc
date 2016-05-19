/*
 * Intel License
 */

#include <random>

#include "webrtc/media/base/videocapturer.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/base/mediaconstraintsimpl.h"
#include "talk/woogeen/sdk/base/webrtcvideorendererimpl.h"
#include "talk/woogeen/sdk/base/customizedframescapturer.h"
#include "woogeen/base/framegeneratorinterface.h"
#include "woogeen/base/stream.h"

namespace woogeen {
namespace base {

Stream::Stream() : media_stream_(nullptr), id_("") {}

Stream::Stream(const std::string& id) : media_stream_(nullptr), id_(id) {}

MediaStreamInterface* Stream::MediaStream() const {
  return media_stream_;
}

Stream::~Stream(){
  if(media_stream_)
    media_stream_->Release();
}

void Stream::MediaStream(MediaStreamInterface* media_stream) {
  RTC_CHECK(media_stream);
  if(media_stream_!=nullptr){
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

void Stream::Attach(VideoRendererARGBInterface& renderer){
  if (media_stream_ == nullptr) {
    ASSERT(false);
    LOG(LS_ERROR) << "Cannot attach an audio only stream to an renderer.";
    return;
  }
  auto video_tracks=media_stream_->GetVideoTracks();
  if(video_tracks.size()==0){
    LOG(LS_ERROR) << "Attach failed because of no video tracks.";
    return;
  }else if (video_tracks.size()>1){
    LOG(LS_WARNING) << "There are more than one video tracks, the first one will be attachecd to renderer.";
  }
  // TODO: delete it when detach or stream is disposed.
  WebrtcVideoRendererARGBImpl* renderer_impl = new WebrtcVideoRendererARGBImpl(renderer);
  rtc::VideoSinkWants wants;
  video_tracks[0]->AddOrUpdateSink(renderer_impl, wants);
  LOG(LS_INFO) << "Attached the stream to a renderer.";
}

LocalStream::LocalStream():media_constraints_(new MediaConstraintsImpl){
}

LocalStream::~LocalStream(){
  delete media_constraints_;
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
    const LocalCameraStreamParameters& parameters) {
  if (!parameters.VideoEnabled() && !parameters.AudioEnabled()) {
    LOG(LS_WARNING) << "Create LocalCameraStream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> pcd_factory =
      PeerConnectionDependencyFactory::Get();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 99999999);
  std::string media_stream_label =
      "MediaStream-" + std::to_string(dis(gen));  // TODO: use UUID.
  scoped_refptr<MediaStreamInterface> stream =
      pcd_factory->CreateLocalMediaStream(media_stream_label);

  // Create video track
  if (parameters.VideoEnabled()) {
    cricket::WebRtcVideoDeviceCapturerFactory factory;
    cricket::VideoCapturer* capturer = nullptr;
    if (!parameters.CameraId().empty()) {
      capturer = factory.Create(cricket::Device(parameters.CameraId(), 0));
    }

    if (!capturer) {
      LOG(LS_WARNING) << "Cannot open the specified camera. Use the default one.";
      std::vector<std::string> device_names;
      {
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
            webrtc::VideoCaptureFactory::CreateDeviceInfo(0));
        if (!info) {
          LOG(LS_ERROR) << "CreateDeviceInfo failed";
        } else {
          int num_devices = info->NumberOfDevices();
          for (int i = 0; i < num_devices; ++i) {
            const uint32_t kSize = 256;
            char name[kSize] = {0};
            char id[kSize] = {0};
            if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
              device_names.push_back(name);
            }
          }
        }
      }

      for (const auto& name : device_names) {
        capturer = factory.Create(cricket::Device(name, 0));
        if (capturer) {
          break;
        }
      }
    }

    if (!capturer) {
      LOG(LS_ERROR) << "WebRtcVideoDeviceCapturerFactory.Create failed";
    } else {
      media_constraints_->SetMandatory(
          webrtc::MediaConstraintsInterface::kMaxWidth,
          std::to_string(parameters.ResolutionWidth()));
      media_constraints_->SetMandatory(
          webrtc::MediaConstraintsInterface::kMaxHeight,
          std::to_string(parameters.ResolutionHeight()));
      media_constraints_->SetMandatory(
          webrtc::MediaConstraintsInterface::kMinWidth,
          std::to_string(parameters.ResolutionWidth()));
      media_constraints_->SetMandatory(
          webrtc::MediaConstraintsInterface::kMinHeight,
          std::to_string(parameters.ResolutionHeight()));

      scoped_refptr<VideoTrackSourceInterface> source =
          pcd_factory->CreateVideoSource(capturer, media_constraints_);
      std::string video_track_label =
          "VideoTrack-" + std::to_string(dis(gen));  // TODO: use UUID.
      scoped_refptr<VideoTrackInterface> video_track =
          pcd_factory->CreateLocalVideoTrack(video_track_label, source);
      stream->AddTrack(video_track);
    }
  }

  // Create audio track
  if (parameters.AudioEnabled()) {
    std::string audio_track_label =
        "AudioTrack-" + std::to_string(dis(gen));  // TODO: use UUID.
    scoped_refptr<AudioTrackInterface> audio_track =
        pcd_factory->CreateLocalAudioTrack(audio_track_label);
    stream->AddTrack(audio_track);
  }

  media_stream_ = stream;
  media_stream_->AddRef();
}

LocalCustomizedStream::~LocalCustomizedStream() {
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
  capturer_ = nullptr;
}

LocalCustomizedStream::LocalCustomizedStream(std::shared_ptr<LocalCustomizedStreamParameters> parameters, VideoFrameGeneratorInterface* framer) {
  if (!parameters->VideoEnabled() && !parameters->AudioEnabled()) {
    LOG(LS_WARNING) << "Create LocalCameraStream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> factory =
      PeerConnectionDependencyFactory::Get();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 99999999);
  std::string media_stream_label =
      "MediaStream-" + std::to_string(dis(gen));
  scoped_refptr<MediaStreamInterface> stream =
      factory->CreateLocalMediaStream(media_stream_label);
  if (parameters->VideoEnabled()) {
    capturer_ = new CustomizedFramesCapturer(framer);
    capturer_->Init();
    scoped_refptr<VideoTrackSourceInterface> source =
        factory->CreateVideoSource(capturer_, NULL);
    std::string video_track_label =
        "VideoTrack-" + std::to_string(dis(gen));
    scoped_refptr<VideoTrackInterface> video_track =
        factory->CreateLocalVideoTrack(video_track_label, source);
    stream->AddTrack(video_track);
  }
  if (parameters->AudioEnabled()) {
    std::string audio_track_label =
        "AudioTrack-" + std::to_string(dis(gen));
    scoped_refptr<AudioTrackInterface> audio_track =
        factory->CreateLocalAudioTrack(audio_track_label);
    stream->AddTrack(audio_track);
  }
  media_stream_ = stream;
  media_stream_->AddRef();
}

RemoteStream::RemoteStream(MediaStreamInterface* media_stream,
                           std::string& from)
    : remote_user_id_(from) {
  media_stream_ = media_stream;
  media_stream_->AddRef();
}

RemoteStream::RemoteStream(std::string& id, std::string& from)
    : Stream(id), remote_user_id_(from) {}

std::string RemoteStream::From() {
  return remote_user_id_;
}

void RemoteStream::MediaStream(
    MediaStreamInterface* media_stream) {
  Stream::MediaStream(media_stream);
}

MediaStreamInterface* RemoteStream::MediaStream() {
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
}
