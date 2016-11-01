/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WOOGEEN_BASE_STREAM_H_
#define WOOGEEN_BASE_STREAM_H_

#include "woogeen/base/exception.h"
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/macros.h"
#include "woogeen/base/videorendererinterface.h"

namespace webrtc {
  class MediaStreamInterface;
  class VideoTrackSourceInterface;
}

namespace woogeen {

namespace conference{
  class ConferencePeerConnectionChannel;
  class ConferenceClient;
}

namespace base {

class MediaConstraintsImpl;

class CustomizedFramesCapturer;

class VideoFrameGeneratorInterface;

using webrtc::MediaStreamInterface;

/// Observer for Stream
class StreamObserver {

};

class WebrtcVideoRendererARGBImpl;

/// Base class of all streams with media stream
class Stream {
  friend class woogeen::conference::ConferencePeerConnectionChannel;

 public:
  /** @cond */
  MediaStreamInterface* MediaStream() const;
  /** @endcond */
  /**
    @brief Get the ID of the stream
    @return Stream's ID
  */
  virtual std::string Id() const;
  /// Disable all audio tracks of the stream.
  virtual void DisableAudio();
  /// Disable all video tracks of the stream.
  virtual void DisableVideo();
  /// Enable all audio tracks of the stream.
  virtual void EnableAudio();
  /// Enable all video tracks of the stream.
  virtual void EnableVideo();
  /// Attach the stream to a renderer.
  WOOGEEN_DEPRECATED virtual void Attach(VideoRendererARGBInterface& renderer){
    AttachVideoRenderer(renderer);
  }
  /// Attach the stream to a renderer.
  virtual void AttachVideoRenderer(VideoRendererARGBInterface& renderer);
  /// Detach the stream from its renderer.
  virtual void DetachVideoRenderer();

 protected:
  Stream(const std::string& id);
  Stream();
  virtual ~Stream();
  void Id(const std::string& id);
  void MediaStream(MediaStreamInterface* media_stream);
  MediaStreamInterface* media_stream_;

 private:
  void SetAudioTracksEnabled(bool enabled);
  void SetVideoTracksEnabled(bool enabled);
  std::string id_;
  WebrtcVideoRendererARGBImpl* renderer_impl_;
};

/**
  @brief This class represents a local stream.
  @detail A local stream can be published to remote side.
*/
class LocalStream : public Stream {
 public:
  LocalStream();
  virtual ~LocalStream();
 protected:
  MediaConstraintsImpl* media_constraints_;
};

/**
  @brief This class represents a remote stream.
  @detail A remote is published from a remote client or MCU. Do not construct
  remote stream outside SDK.
*/
class RemoteStream : public Stream {
  friend class woogeen::conference::ConferencePeerConnectionChannel;
  friend class woogeen::conference::ConferenceClient;

 public:
  /// Return the remote user ID, indicates who published this stream.
  std::string From();

 protected:
  explicit RemoteStream(std::string& id, std::string& from);
  explicit RemoteStream(MediaStreamInterface* media_stream, std::string& from);

  MediaStreamInterface* MediaStream();
  void MediaStream(MediaStreamInterface* media_stream);

 private:
  std::string remote_user_id_;
  // Indicates whether this stream can be subscribed with audio or video.
  // It does NOT mean the media stream has audio/video tracks.
  bool has_audio_ = true;
  bool has_video_ = true;
};

/// This class represents a remote stream captured from a camera and/or mic.
class RemoteCameraStream : public RemoteStream {
 public:
  /** @cond */
  explicit RemoteCameraStream(std::string& id, std::string& from);
  explicit RemoteCameraStream(MediaStreamInterface* media_stream,
                              std::string& from);
  /** @endcond */
};

/// This class represents a remote stream captured from screen sharing.
class RemoteScreenStream : public RemoteStream {
 public:
  /** @cond */
  explicit RemoteScreenStream(std::string& id, std::string& from);
  explicit RemoteScreenStream(MediaStreamInterface* media_stream,
                              std::string& from);
  /** @endcond */
};

/// This class represents a local stream captured from camera, mic.
class LocalCameraStream : public LocalStream {
 public:
  /**
    Create a LocalCameraStream with parameters.
    @param parameters Parameters for creating the stream. The stream will not be
    impacted if chaning parameters after it is created.
    @param error_code If creation successes, it will be 0, otherwise, it will be
    the error code occurred.
    @return Returns a shared pointer for created stream. Returns nullptr if
    failed.
  */
  static std::shared_ptr<LocalCameraStream> Create(
      const LocalCameraStreamParameters& parameters,
      int& error_code);
  /** @cond */
  /**
    Create a RTCLocalCameraStream with specific video source.
    @param is_audio_enabled Indicates whether audio is enabled.
    @param video_source LocalCameraStream created will have a video track use
    |video_source| as its source. Changing |video_source| will impact the video
    track in current stream.
    @param error_code If creation successes, it will be 0, otherwise, it will be
    the error code occurred.
    @return Returns a shared pointer for created stream. Returns nullptr if
    failed.
  */
  static std::shared_ptr<LocalCameraStream> Create(
      const bool is_audio_enabled,
      /* Consider to change this parameter to be a reference */
      webrtc::VideoTrackSourceInterface* video_source,
      int& error_code);
  /**
    Initialize a LocalCameraStream with parameters.
    @param parameters Parameters for creating the stream. The stream will not be
    impacted if chaning parameters after it is created.
  */
  WOOGEEN_DEPRECATED explicit LocalCameraStream(
      const LocalCameraStreamParameters& parameters);
  ~LocalCameraStream();
  /**
    @brief Close the stream. Its underlying media source is no longer providing
    data, and will never provide more data for this stream.
    @detail Once a stream is closed, it is no longer usable. If you want to
    temporary disable audio or video, please use DisableAudio/DisableVideo
    instead.
  */
  void Close();
protected:
 explicit LocalCameraStream(const LocalCameraStreamParameters& parameters,
                            int& error_code);
 explicit LocalCameraStream(const bool is_audio_enabled,
                            webrtc::VideoTrackSourceInterface* video_source,
                            int& error_code);
 /** @endcond */
};

/// This class represents a local stream which use frame generator to generate frames.
class LocalCustomizedStream : public LocalStream {
  public:
  /**
    Initialize a LocalCustomizedStream with parameters.
    @param parameters Parameters for creating the stream. The stream will not be
    impacted if chaning parameters after it is created.
    @param framer An instance implemented VideoFrameGeneratorInterface.
  */
   explicit LocalCustomizedStream(
       std::shared_ptr<LocalCustomizedStreamParameters> parameters,
       VideoFrameGeneratorInterface* framer);
   ~LocalCustomizedStream();

  private:
   CustomizedFramesCapturer* capturer_;
};

} // namespace base
} // namespace woogeen

#endif  // WOOGEEN_BASE_STREAM_H_
