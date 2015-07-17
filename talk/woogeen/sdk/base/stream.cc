/*
 * Intel License
 */

#include "talk/media/base/videocapturer.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/woogeen/sdk/base/stream.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"

namespace woogeen {

Stream::Stream() : id_(""){
}

Stream::Stream(std::string& id) : id_(id){
}

scoped_refptr<MediaStreamInterface> Stream::MediaStream() {
  CHECK(media_stream_);
  return media_stream_;
}

void Stream::MediaStream(scoped_refptr<MediaStreamInterface> media_stream) {
  CHECK(media_stream);
  media_stream_=media_stream;
}

std::string& Stream::Id(){
  return id_;
}

void Stream::Id(std::string& id){
  id_=id;
}

LocalCameraStream::~LocalCameraStream(){
  LOG(LS_INFO) << "Destory LocalCameraStream.";
}

LocalCameraStream::LocalCameraStream(std::shared_ptr<LocalCameraStreamParameters> parameters) {
  if(!parameters->VideoEnabled()&&!parameters->AudioEnabled()){
    LOG(LS_WARNING) << "Create LocalCameraStream without video and audio.";
  }
  scoped_refptr<PeerConnectionDependencyFactory> factory = PeerConnectionDependencyFactory::Get();
  std::string media_stream_label = "MediaStream";
  scoped_refptr<MediaStreamInterface> stream = factory->CreateLocalMediaStream(media_stream_label);
  if(parameters->VideoEnabled()){
    rtc::scoped_ptr<cricket::DeviceManagerInterface> device_manager(cricket::DeviceManagerFactory::Create());
    bool initialized = device_manager->Init();
    cricket::Device device;
    if (!device_manager->GetVideoCaptureDevice(parameters->CameraId(), &device)) {
      LOG(LS_ERROR) << "GetVideoCaptureDevice failed";
      return;
    }
    rtc::scoped_ptr<cricket::VideoCapturer> capturer(device_manager->CreateVideoCapturer(device));
    media_constraints_.SetMandatory(webrtc::MediaConstraintsInterface::kMaxWidth, std::to_string(parameters->ResolutionWidth()));
    media_constraints_.SetMandatory(webrtc::MediaConstraintsInterface::kMaxHeight, std::to_string(parameters->ResolutionHeight()));
    scoped_refptr<VideoSourceInterface> source = factory->CreateVideoSource(capturer.get(), &media_constraints_);
    std::string video_track_label = "VideoTrack";
    scoped_refptr<VideoTrackInterface> video_track = factory->CreateLocalVideoTrack(video_track_label, source);
    stream->AddTrack(video_track);
    capturer_.reset(capturer.release());
  }
  if(parameters->AudioEnabled()){
    std::string audio_track_label = "AudioTrack";
    scoped_refptr<AudioTrackInterface> audio_track = factory->CreateLocalAudioTrack(audio_track_label);
    stream->AddTrack(audio_track);
  }
  media_stream_=stream;
}

RemoteStream::RemoteStream(MediaStreamInterface* media_stream, std::string& from)
    : remote_user_id_(from){
  media_stream_=media_stream;
}

RemoteStream::RemoteStream(std::string& id, std::string& from)
    : Stream(id),
      remote_user_id_(from) {
}
std::string& RemoteStream::From(){
  return remote_user_id_;
}

void RemoteStream::MediaStream(scoped_refptr<MediaStreamInterface> media_stream){
  media_stream_=media_stream;
}

scoped_refptr<MediaStreamInterface> RemoteStream::MediaStream() {
  return media_stream_;
}

RemoteCameraStream::RemoteCameraStream(std::string& id, std::string& from) : RemoteStream(id, from) {
}

RemoteCameraStream::RemoteCameraStream(MediaStreamInterface* media_stream, std::string& from) : RemoteStream(media_stream, from) {
}

RemoteScreenStream::RemoteScreenStream(std::string& id, std::string& from) : RemoteStream(id, from) {
}

RemoteScreenStream::RemoteScreenStream(MediaStreamInterface* media_stream, std::string& from) : RemoteStream(media_stream, from) {
}

}
