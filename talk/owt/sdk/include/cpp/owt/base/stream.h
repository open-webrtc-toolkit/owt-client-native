// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_STREAM_H_
#define OWT_BASE_STREAM_H_
#include <mutex>
#include <unordered_map>
#include <vector>
#include "owt/base/commontypes.h"
#include "owt/base/exception.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/macros.h"
#include "owt/base/options.h"
#include "owt/base/videoencoderinterface.h"
#include "owt/base/videorendererinterface.h"
namespace webrtc {
  class MediaStreamInterface;
  class VideoTrackSourceInterface;
}
namespace owt {
namespace conference {
  class ConferencePeerConnectionChannel;
  class ConferenceClient;
  class ConferenceInfo;
}
namespace p2p {
  class P2PPeerConnectionChannel;
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
  /// Triggered when the stream info is updated in conference mode.
  virtual void OnUpdated() {};
};
class WebrtcVideoRendererImpl;
#if defined(WEBRTC_WIN)
class WebrtcVideoRendererD3D9Impl;
#endif
/// Base class of all streams with media stream
class Stream {
 public:
  Stream(MediaStreamInterface* media_stream, StreamSourceInfo source);
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
  /// Attach the stream to a renderer to receive ARGB/I420 frames for local or remote stream.
  /// Be noted if you turned hardware acceleration on, calling this API on remote stream
  /// will have no effect.
  virtual void AttachVideoRenderer(VideoRendererInterface& renderer);
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
  virtual StreamSourceInfo Source() const;
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
  void TriggerOnStreamUpdated();
  MediaStreamInterface* media_stream_;
  std::unordered_map<std::string, std::string> attributes_;
  WebrtcVideoRendererImpl* renderer_impl_;
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
    virtual ~LocalScreenStreamObserver() {}
};
/**
  @brief This class represents a local stream.
  @details A local stream can be published to remote side.
*/
class LocalStream : public Stream {
 public:
#if !defined(WEBRTC_WIN)
  LocalStream();
  LocalStream(MediaStreamInterface* media_stream, StreamSourceInfo source);
#endif
  virtual ~LocalStream();
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
  /**
   @brief Close a local stream
   @details This will remote all tracks on the stream, and detach any
   sink previously attached.
  */
  void Close();
  /**
   @brief Create a local camera stream.
   @detail This creates a local camera stream with specified device
   settings.
   @param parameters Local camera stream settings for stream creation.
   @param error_code Error code will be set if creation fails.
   @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      const LocalCameraStreamParameters& parameters,
      int& error_code);
  /**
   @brief Create a local camera stream with video track source.
   @detail This creates a local camera stream with specified video track source.
   @param is_audio_enabled If audio is enabled in the stream.
   @param video_source Pointer to VideoTrackSourceInterface used for stream
   creation.
   @param error_code Error code will be set if creation fails.
   @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      const bool is_audio_enabled,
      webrtc::VideoTrackSourceInterface* video_source,
      int& error_code);
  /**
    @brief Initialize a LocalCustomizedStream with parameters and frame generator.
    @details The input of the video stream MUST be YUV frame if initializing with frame
    generator.
    @param parameters Parameters for creating the stream. The stream will not
    be impacted if changing parameters after it is created.
    @param framer Pointer to an instance implemented VideoFrameGeneratorInterface.
    This instance will be destroyed by SDK when stream is closed.
    @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer);
  /**
    @briefInitialize a local customized stream with parameters and encoder interface.
    @details The input of the video stream MUST be encoded frame if initializing with
    video encoder interface.
    @param parameters Parameters for creating the stream. The stream will not
    be impacted if changing parameters after it is created.
    @param encoder Pointer to an instance implementing VideoEncoderInterface.
    @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface* encoder);
#if defined(WEBRTC_WIN)
  /**
    @brief Initialize a local screen stream with parameters.
    @param parameters Parameters for creating the stream. The stream will
    not be impacted if changing parameters after it is created.
    @param observer Source callback required for application to specify capture
    window/screen source.
    @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      std::shared_ptr<LocalDesktopStreamParameters> parameters,
      std::unique_ptr<LocalScreenStreamObserver> observer);
#endif
 protected:
     explicit LocalStream(const LocalCameraStreamParameters& parameters,
         int& error_code);
     explicit LocalStream(const bool is_audio_enabled,
         webrtc::VideoTrackSourceInterface* video_source,
         int& error_code);
     explicit LocalStream(
         std::shared_ptr<LocalCustomizedStreamParameters> parameters,
         std::unique_ptr<VideoFrameGeneratorInterface> framer);
     explicit LocalStream(
         std::shared_ptr<LocalCustomizedStreamParameters> parameters,
         VideoEncoderInterface* encoder);
#if defined(WEBRTC_WIN)
     explicit LocalStream(
         std::shared_ptr<LocalDesktopStreamParameters> parameters,
         std::unique_ptr<LocalScreenStreamObserver> observer
     );
#endif
    MediaConstraintsImpl* media_constraints_;
private:
    bool encoded_ = false;
#if defined(WEBRTC_MAC)
    std::unique_ptr<ObjcVideoCapturerInterface> capturer_;
#endif
};
/**
  @brief This class represents a remote stream.
  @details A remote is published from a remote client or MCU. Do not construct
  remote stream outside SDK.
*/
class RemoteStream : public Stream {
  friend class owt::conference::ConferencePeerConnectionChannel;
  friend class owt::conference::ConferenceClient;
  friend class owt::conference::ConferenceInfo;
  friend class owt::p2p::P2PPeerConnectionChannel;
 public:
  /** @cond */
  explicit RemoteStream(const std::string& id,
                        const std::string& from,
                        const owt::base::SubscriptionCapabilities& subscription_capabilities,
                        const owt::base::PublicationSettings& publication_settings);
  explicit RemoteStream(MediaStreamInterface* media_stream,
                        const std::string& from);
  virtual void Attributes(const std::unordered_map<std::string, std::string>& attributes) {
                          attributes_ = attributes;
  }
  /** @endcond */
  /// Return the remote user ID, indicates who published this stream.
  /// If it's mixed stream, origin will be "mcu".
  std::string Origin();
  using Stream::Attributes;
  /// Get the subscription capabilities on the stream.
  SubscriptionCapabilities Capabilities() { return subscription_capabilities_; }
  /// Get the publication settings of the stream.
  PublicationSettings Settings() { return publication_settings_; }
  /// Stop the remote stream.
  /** @cond */
  /// Setter for subscription capabilities
  void Capabilities(
      owt::base::SubscriptionCapabilities subscription_capabilities) {
    subscription_capabilities_ = subscription_capabilities;
  }
  /// Setter for publication settings
  void Settings(PublicationSettings publication_settings) {
    publication_settings_ = publication_settings;
  }
  /** @endcond */
  void Stop() {};
 protected:
  MediaStreamInterface* MediaStream();
  void MediaStream(MediaStreamInterface* media_stream);
 private:
  std::string origin_;
  bool has_audio_ = true;
  bool has_video_ = true;
  owt::base::SubscriptionCapabilities subscription_capabilities_;
  owt::base::PublicationSettings publication_settings_;
};
} // namespace base
} // namespace owt
#endif  // OWT_BASE_STREAM_H_
