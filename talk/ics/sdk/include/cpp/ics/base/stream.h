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

#ifndef ICS_BASE_STREAM_H_
#define ICS_BASE_STREAM_H_

#include <mutex>
#include <unordered_map>
#include <vector>

#include "ics/base/commontypes.h"
#include "ics/base/exception.h"
#include "ics/base/localcamerastreamparameters.h"
#include "ics/base/macros.h"
#include "ics/base/options.h"
#include "ics/base/videoencoderinterface.h"
#include "ics/base/videorendererinterface.h"

namespace webrtc {
  class MediaStreamInterface;
  class VideoTrackSourceInterface;
}

namespace ics {
namespace conference{
  class ConferencePeerConnectionChannel;
  class ConferenceClient;
  class ConferenceInfo;
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
 public:
  /// Triggered when a stream is ended, or the stream is no longer available in
  /// conference mode.
  virtual void OnEnded() {};
};

class WebrtcVideoRendererARGBImpl;

#if defined(WEBRTC_WIN)
class WebrtcVideoRendererD3D9Impl;
#endif

/// Base class of all streams with media stream
class Stream {
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
  /**
    @brief Returns the audio/video source info of the stream
    @details The source info of video/audio indicates the device source type of
    video/audio track source. For conference mode, if the video/audio track is
    from mixed stream, it will be set as kMixed.
  */
  virtual StreamSourceInfo SourceInfo() const;
#if defined(WEBRTC_WIN)
  /// Attach the stream to a renderer to receive frames from decoder.
  /// Both I420 frame and native surface is supported.
  virtual void AttachVideoRenderer(VideoRenderWindow& render_window);
#endif
  /// Detach the stream from its renderer.
  virtual void DetachVideoRenderer();
  /// Register an observer on the stream.
  void AddObserver(StreamObserver& observer);
  /// De-Register an observer on the stream.
  void RemoveObserver(StreamObserver& observer);
 protected:
  Stream(const std::string& id);
  Stream();
  virtual ~Stream();
  void Id(const std::string& id);
  void MediaStream(MediaStreamInterface* media_stream);
  void TriggerOnStreamEnded();
  MediaStreamInterface* media_stream_;
  std::unordered_map<std::string, std::string> attributes_;
  WebrtcVideoRendererARGBImpl* renderer_impl_;
#if defined(WEBRTC_WIN)
  WebrtcVideoRendererD3D9Impl* d3d9_renderer_impl_;
#endif
  StreamSourceInfo source_;
 private:
  void SetAudioTracksEnabled(bool enabled);
  void SetVideoTracksEnabled(bool enabled);
  bool ended_;
  std::string id_;
  mutable std::mutex observer_mutex_;
  std::vector<std::reference_wrapper<StreamObserver>> observers_;
};

/**
  @brief This class represents a local stream.
  @details A local stream can be published to remote side.
*/
class LocalStream : public Stream {
 public:
  LocalStream();
  virtual ~LocalStream();
  /** @cond */

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
  friend class ics::conference::ConferencePeerConnectionChannel;
  friend class ics::conference::ConferenceClient;
  friend class ics::conference::ConferenceInfo;
 public:
  /// Return the remote user ID, indicates who published this stream.
  /// If it's mixed stream, origin will be "mcu".
  std::string Origin();
  using Stream::Attributes;
  SubscriptionCapabilities Capabilities() { return subscription_capabilities_; }
  PublicationSettings Settings() { return publication_settings_; }

  void Stop() {};

 protected:
  /** @cond */
  explicit RemoteStream(const std::string& id,
                        const std::string& from,
                        const ics::base::SubscriptionCapabilities& subscription_capabilities,
                        const ics::base::PublicationSettings& publication_settings);
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
  std::string origin_;
  bool has_audio_ = true;
  bool has_video_ = true;
  ics::base::SubscriptionCapabilities subscription_capabilities_;
  ics::base::PublicationSettings publication_settings_;
};

/// This class represents a remote stream captured from a camera and/or mic.00000000000000000000000
class RemoteCameraStream : public RemoteStream {
 public:
  /** @cond */
  explicit RemoteCameraStream(std::string& id, std::string& from,
                              const ics::base::SubscriptionCapabilities& subscription_capabilities,
                              const ics::base::PublicationSettings& publication_settings);
  explicit RemoteCameraStream(MediaStreamInterface* media_stream,
                              std::string& from);
  /** @endcond */
};

/// This class represents a remote stream captured from screen sharing.
class RemoteScreenStream : public RemoteStream {
 public:
  /** @cond */
  explicit RemoteScreenStream(std::string& id, std::string& from,
                              const ics::base::SubscriptionCapabilities& subscription_capabilities,
                              const ics::base::PublicationSettings& publication_settings);
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
    Create a ICSLocalCameraStream with specific video source.
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

class LocalScreenStreamObserver {
 public:
  /**
    @brief Event callback for local screen stream to request for a source from application.
    @details After local stream is started, this callback will be invoked to request for
    a source from application.
    @param window_list list of windows/screen's (id, title) pair.
    @param dest_window application will set this id to be used by it.
  */
  virtual void OnCaptureSourceNeeded(const std::unordered_map<int, std::string>& window_list, int& dest_window) {}
};

/// This class represents a local stream which uses local screen/app to generate
/// frames.
class LocalScreenStream : public LocalStream {
 public:
  /**
    @brief Initialize a LocalScreenStream with parameters.
    @param parameters Parameters for creating the stream. The stream will
    not be impacted if changing parameters after it is created.
    @param observer Source callback required for application to specify capture window/screen source.
  */
  explicit LocalScreenStream(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer);
  virtual ~LocalScreenStream();
};

} // namespace base
} // namespace ics

#endif  // ICS_BASE_STREAM_H_
