/*
 * Copyright © 2017 Intel Corporation. All Rights Reserved.
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

#include <unordered_map>
#include "woogeen/base/exception.h"
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/macros.h"
#include "woogeen/base/videoencoderinterface.h"
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

class BasicDesktopCapturer;

class VideoFrameGeneratorInterface;

#if defined(WEBRTC_MAC)
class ObjcVideoCapturerInterface;
#endif

using webrtc::MediaStreamInterface;

/// Observer for Stream
class StreamObserver {

};

class WebrtcVideoRendererARGBImpl;

#if defined(WEBRTC_WIN)
class WebrtcVideoRendererD3D9Impl;
#endif

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
  /// Attach the stream to a renderer to receive ARGB frames for local or remote stream.
  /// Be noted if you turned hardware acceleration on, calling this API on remote stream
  /// will have no effect.
  virtual void AttachVideoRenderer(VideoRendererARGBInterface& renderer);

  /**
    @brief Returns a user-defined attribute map.
    @details These attributes are defined by publisher. P2P mode always return
    empty map because it is not supported yet.
  */
  virtual const std::unordered_map<std::string, std::string> Attributes()
      const {
    return attributes_;
  }

#if defined(WEBRTC_WIN)
  /// Attach the stream to a renderer to receive frames from decoder.
  /// Both I420 frame and native surface is supported.
  virtual void AttachVideoRenderer(VideoRenderWindow& render_window);
#endif
  /// Detach the stream from its renderer.
  virtual void DetachVideoRenderer();

 protected:
  Stream(const std::string& id);
  Stream();
  virtual ~Stream();
  void Id(const std::string& id);
  void MediaStream(MediaStreamInterface* media_stream);
  MediaStreamInterface* media_stream_;
  std::unordered_map<std::string, std::string> attributes_;
  WebrtcVideoRendererARGBImpl* renderer_impl_;
#if defined(WEBRTC_WIN)
  WebrtcVideoRendererD3D9Impl* d3d9_renderer_impl_;
#endif
 private:
  void SetAudioTracksEnabled(bool enabled);
  void SetVideoTracksEnabled(bool enabled);
  std::string id_;
};

/**
  @brief This class represents a local stream.
  @details A local stream can be published to remote side.
*/
class LocalStream : public Stream {
 public:
  enum class StreamDeviceType : int {
    // TODO(jianlin): For now we treat customized raw/encoded input as
    // camera device. Better has clear device type negotation with MCU
    // for this.
    kStreamDeviceTypeCamera = 101,
    kStreamDeviceTypeScreen,
    kStreamDeviceTypeUnknown = 200
  };
  LocalStream();
  virtual ~LocalStream();
  /** @cond */
  virtual StreamDeviceType GetStreamDeviceType() {
    return StreamDeviceType::kStreamDeviceTypeUnknown;
  }

  using Stream::Attributes;
  /**
    @brief Set a user-defined attribute map.
    @details Remote user can get attribute map by calling Attributes(). P2P mode
    does not support setting attributes.
  */
  virtual void Attributes(
      const std::unordered_map<std::string, std::string>& attributes) {
    attributes_ = attributes;
  }
  /** @endcond */
 protected:
  MediaConstraintsImpl* media_constraints_;
};

/**
  @brief This class represents a remote stream.
  @details A remote is published from a remote client or MCU. Do not construct
  remote stream outside SDK.
*/
class RemoteStream : public Stream {
  friend class woogeen::conference::ConferencePeerConnectionChannel;
  friend class woogeen::conference::ConferenceClient;

 public:
  /// Return the remote user ID, indicates who published this stream.
  std::string From();
  using Stream::Attributes;

 protected:
  /** @cond */
  explicit RemoteStream(const std::string& id, const std::string& from);
  explicit RemoteStream(MediaStreamInterface* media_stream,
                        const std::string& from);

  virtual void Attributes(
      const std::unordered_map<std::string, std::string>& attributes) {
    attributes_ = attributes;
  }
  /** @endcond */

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
  /** @endcond */
  /**
    @brief Close the stream. Its underlying media source is no longer providing
    data, and will never provide more data for this stream.
    @details Once a stream is closed, it is no longer usable. If you want to
    temporary disable audio or video, please use DisableAudio/DisableVideo
    instead.
  */
  void Close();
  /** @cond */
  StreamDeviceType GetStreamDeviceType() final {
    return StreamDeviceType::kStreamDeviceTypeCamera;
  }

 protected:
  explicit LocalCameraStream(const LocalCameraStreamParameters& parameters,
                             int& error_code);
  explicit LocalCameraStream(const bool is_audio_enabled,
                             webrtc::VideoTrackSourceInterface* video_source,
                             int& error_code);
  /** @endcond */

 private:
#if defined(WEBRTC_MAC)
  std::unique_ptr<ObjcVideoCapturerInterface> capturer_;
#endif
};

/// This class represents a local stream which uses frame generator to generate
//  I420 frames or depends on a video encoder to generate encoded frames.
class LocalCustomizedStream : public LocalStream {
  public:
   /**
     Initialize a LocalCustomizedStream with parameters and frame generator.

     The input of the video stream MUST be YUV frame if initializing with frame
     generator.

     @param parameters Parameters for creating the stream. The stream will not
     be impacted if changing parameters after it is created.
     @param framer Pointer to an instance implemented VideoFrameGeneratorInterface.
     This instance will be destroyed by SDK when stream is closed.
   */
   explicit LocalCustomizedStream(
       std::shared_ptr<LocalCustomizedStreamParameters> parameters,
       std::unique_ptr<VideoFrameGeneratorInterface> framer);
   /**
     Initialize a LocalCustomizedStream with parameters and encoder interface.

     The input of the video stream MUST be encoded frame if initializing with
     video encoder interface.

     @param parameters Parameters for creating the stream. The stream will not
     be impacted if changing parameters after it is created.
     @param encoder Pointer to an instance implementing VideoEncoderInterface.
   */
   explicit LocalCustomizedStream(
       std::shared_ptr<LocalCustomizedStreamParameters> parameters,
       VideoEncoderInterface* encoder);
   ~LocalCustomizedStream();
   /** @cond */
   // Temporarily use camera type for customized stream.
   StreamDeviceType GetStreamDeviceType() final {
     return StreamDeviceType::kStreamDeviceTypeCamera;
   }
   /** @endcond */

   /// Attach the stream to a renderer to receive ARGB frames from decoder.
   void AttachVideoRenderer(VideoRendererARGBInterface& renderer);
#if defined(WEBRTC_WIN)
   /// Attach the stream to a renderer to receive frames from decoder.
   /// Both I420 frame and native surface is supported.
   void AttachVideoRenderer(VideoRenderWindow& render_window);
#endif
   /// Detach the stream from its renderer.
   void DetachVideoRenderer();
  private:
   bool encoded_ = false;
};

/// This class represents a local stream which uses local screen/app to generate
/// frames.
class LocalScreenStream : public LocalStream {
 public:
  /**
    Initialize a LocalScreenStream with parameters.
    @param parameters Parameters for creating the stream. The stream will
    not be impacted if changing parameters after it is created.
  */
  explicit LocalScreenStream(
      std::shared_ptr<LocalDesktopStreamParameters> parameters);
  virtual ~LocalScreenStream();
  /**
    Get a list of (id, title) pair that enumerates apps/screen available for
    sharing.
    @param source_list the caller provided map which will be filled with
    available sources to capture from.
    @return Return true if successfully get the list; Return false if fail
    to get the list, or stream device type is screen.
  */
  bool GetCurrentSourceList(std::unordered_map<int, std::string>* source_list);
  /**
    Set the window to be captured from if current source type is application.
    @param source_id The id retrieved from previous GetCurentSourceList()
    call. Note the window id might be invalid when calling
    SetCurrentCaptureSource().
    @return Returns true if set the capture window target successfully.Returns
    false on failing to set the capture window target, or stream device type is
    screen.
  */
  bool SetCurrentCaptureSource(int source_id);
  /** @cond */
  StreamDeviceType GetStreamDeviceType() final {
    return StreamDeviceType::kStreamDeviceTypeScreen;
  }
  /** @endcond */
};

} // namespace base
} // namespace woogeen

#endif  // WOOGEEN_BASE_STREAM_H_
