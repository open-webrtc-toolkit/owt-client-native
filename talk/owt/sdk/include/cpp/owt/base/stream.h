// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_STREAM_H_
#define OWT_BASE_STREAM_H_
#include <cstdin>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "owt/base/commontypes.h"
#include "owt/base/exception.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/macros.h"
#include "owt/base/options.h"
#include "owt/base/videoencoderinterface.h"
#include "owt/base/audioplayerinterface.h"
#ifdef OWT_ENABLE_QUIC
#include "owt/quic/quic_transport_stream_interface.h"
#endif
namespace webrtc {
class MediaStreamInterface;
class VideoTrackSourceInterface;
class MediaConstraints;
}  // namespace webrtc

namespace owt {
namespace conference {
class ConferencePeerConnectionChannel;
class ConferenceClient;
class ConferenceInfo;
}  // namespace conference
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
class VideoRenderWindow;
class VideoRendererInterface;
using webrtc::MediaStreamInterface;
/// Observer for Stream
class StreamObserver {
 public:
  virtual ~StreamObserver() = default;
  /// Triggered when a stream is ended, or the stream is no longer available in
  /// conference mode.
  virtual void OnEnded() {}
  /// Triggered when the stream info is updated in conference mode.
  virtual void OnUpdated() {}
  /// Triggered when the stream is muted
  virtual void OnMute(TrackKind track_kind) {}
  /// Triggered when the stream is unmuted
  virtual void OnUnmute(TrackKind track_kind) {}
};
class WebrtcVideoRendererImpl;
class WebrtcAudioRendererImpl;
#if defined(WEBRTC_WIN)
class WebrtcVideoRendererD3D9Impl;
#endif
/// Base class of all streams with media stream
class Stream {
  friend class owt::conference::ConferenceClient;

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
  /**
    @brief Returns a user-defined attribute map.
    @details These attributes are defined by publisher. P2P mode always return
    empty map because it is not supported yet.
  */
  virtual const std::unordered_map<std::string, std::string> Attributes()
      const {
    return attributes_;
  }
#ifdef OWT_ENABLE_QUIC
  /**
   @brief Check if current stream is used for WebTransport
   @return True if current stream is used for WebTransport. False otherwise.
   */
  virtual bool DataEnabled() const {
    return is_data_;
  }
#endif
  /**
    @brief Returns the audio/video source info of the stream
    @details The source info of video/audio indicates the device source type of
    video/audio track source. For conference mode, if the video/audio track is
    from mixed stream, it will be set as kMixed.
  */
  virtual StreamSourceInfo Source() const;
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /// Attach the stream to a renderer to receive ARGB/I420 frames for local or
  /// remote stream. Be noted if you turned hardware acceleration on, calling
  /// this API on remote stream will have no effect.
  virtual void AttachVideoRenderer(VideoRendererInterface& renderer);
  /// Attach the stream to an audio player that receives PCM data besides sending to
  /// audio output device.
  virtual void AttachAudioPlayer(AudioPlayerInterface& player);
#endif
#if defined(WEBRTC_WIN)
  /// Attach the stream to a renderer to receive frames from decoder.
  /// Both I420 frame and native surface is supported.
  virtual void AttachVideoRenderer(VideoRenderWindow& render_window);
#endif
  /// Detach the stream from its renderer.
  virtual void DetachVideoRenderer();
  /// Detach the stream from the audio player.
  virtual void DetachAudioPlayer();
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
  void TriggerOnStreamMute(owt::base::TrackKind track_kind);
  void TriggerOnStreamUnmute(owt::base::TrackKind track_kind);
  MediaStreamInterface* media_stream_;
  std::unordered_map<std::string, std::string> attributes_;
  WebrtcVideoRendererImpl* renderer_impl_;
  WebrtcAudioRendererImpl* audio_renderer_impl_;
#if defined(WEBRTC_WIN)
  WebrtcVideoRendererD3D9Impl* d3d9_renderer_impl_;
#endif
  StreamSourceInfo source_;
#ifdef OWT_ENABLE_QUIC
  bool is_data_ = false;
#endif
 private:
  void SetAudioTracksEnabled(bool enabled);
  void SetVideoTracksEnabled(bool enabled);
  bool ended_;
  std::string id_;
  mutable std::mutex observer_mutex_;
  std::vector<std::reference_wrapper<StreamObserver>> observers_;
};

#ifdef OWT_ENABLE_QUIC
///  A writable stream  is created by MCU which can be used to construct
///  LocalStream for publishing.
class WritableStream : public owt::quic::QuicTransportStreamInterface::Visitor {
 public:
  WritableStream(owt::quic::QuicTransportStreamInterface* quic_stream,
                 const std::string& session_id)
      : quic_stream_(quic_stream), session_id_(session_id) {
    if (quic_stream_) {
      quic_stream_->SetVisitor(this);
    }
  }
  ~WritableStream() {
    if (quic_stream_) {
      delete quic_stream_;
      quic_stream_ = nullptr;
    }
  }
  void Write(uint8_t* data, size_t length) {
    if (quic_stream_ && can_write_ && !fin_read_) {
      quic_stream_->Write(data, length);
    }
  }
  // Overrides QuicTransportStreamInterface::Visitor
  void OnCanRead() override {
    can_read_ = true;
  }
  void OnCanWrite() override {
    // According to transport payload protocol, send the initial session id.
    quic_stream_->Write(session_id_.c_str(), session_id_.length());
    can_write_ = true;
  }
  void OnFinRead() override {
    fin_read_ = true;
  }
 private:
  // Writable stream is always created by conference client with the underlying QuicStream owned
  // by current WritableStream.
  owt::quic::QuicTransportStreamInterface* quic_stream_;
  std::string session_id_;
  std::atomic<bool> can_read_ = false;
  std::atomic<bool> can_write_ = false;
  std::atomic<bool> fin_read_ = false;
};

/// <summary>
///  A readable stream constructed from QuicStream for remote webtransport
///  stream from MCU.
/// </summary>
class ReadableStream : public owt::quic::QuicTransportStreamInterface::Visitor {
 public:
  ReadableStream(owt::quic::QuicTransportStreamInterface* quic_stream,
                 const std::string& session_id)
      : quic_stream_(quic_stream), session_id_(session_id) {
    if (quic_stream_) {
      quic_stream_->SetVisitor(this);
    }
  }
  ~ReadableStream() {
    if (quic_stream_) {
      delete quic_stream_;
      quic_stream_ = nullptr;
    }
  }
  size_t Read(uint8_t* data, size_t length) {
    if (quic_stream_ && data && length > 0 && can_read && !fin_read_) {
      return quic_stream_->Read(data, length);
    } else {
      return 0;
    }
  }
  size_t ReadableBytes const {
    if (quic_stream_ && can_read && !fin_read_) {
      return quic_stream_->ReadableBytes();
    } else {
      return 0;
    }
  }
  // Overrides QuicTransportStreamInterface::Visitor
  void OnCanRead() override {
    can_read_ = true;
  }
  void OnCanWrite() override {
    quic_stream_->Write(session_id_.c_str(), session_id_.length());
    can_write_ = true;
  }
  void OnFinRead() override {
    fin_read_ = true;
  }
 private:
  owt::quic::QuicTransportStreamInterface* quic_stream_;
  std::string session_id_;
  std::atomic<bool> can_read_ = false;
  std::atomic<bool> can_write_ = false;
  std::atomic<bool> fin_read_ = false;
};

class LocalScreenStreamObserver {
 public:
  /**
  @brief Event callback for local screen stream to request for a source from
  application.
  @details After local stream is started, this callback will be invoked to
  request for a source from application.
  @param window_list list of windows/screen's (id, title) pair.
  @param dest_window application will set this id to be used by it.
  */
  virtual void OnCaptureSourceNeeded(
      const std::unordered_map<int, std::string>& window_list,
      int& dest_window) {}
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

#ifdef OWT_ENABLE_QUIC
  /**
   @brief Create a data stream.
   @detail This creates a WebTransport stream with specified configuraitons.
   @param parameters WebTransport stream settings for stream creation.
   @param error_code Error code will be set if creation fails.
  */
  static std::shared_ptr<LocalStream> Create(
      std::shared_ptr<WritableStream> writable_stream,
      int& error_code);
#endif

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
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /**
    @brief Initialize a LocalCustomizedStream with parameters and frame
    generator.
    @details The input of the video stream MUST be YUV frame if initializing
    with frame generator.
    @param parameters Parameters for creating the stream. The stream will not
    be impacted if changing parameters after it is created.
    @param framer Pointer to an instance implemented
    VideoFrameGeneratorInterface. This instance will be destroyed by SDK when
    stream is closed.
    @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer);
  /**
    @briefInitialize a local customized stream with parameters and encoder
    interface.
    @details The input of the video stream MUST be encoded frame if initializing
    with video encoder interface.
    @param parameters Parameters for creating the stream. The stream will not
    be impacted if changing parameters after it is created.
    @param encoder Pointer to an instance implementing VideoEncoderInterface.
    @return Pointer to created LocalStream.
  */
  static std::shared_ptr<LocalStream> Create(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface* encoder);
#endif

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
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  explicit LocalStream(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      std::unique_ptr<VideoFrameGeneratorInterface> framer);
  explicit LocalStream(
      std::shared_ptr<LocalCustomizedStreamParameters> parameters,
      VideoEncoderInterface* encoder);
#endif
#if defined(WEBRTC_WIN)
  explicit LocalStream(std::shared_ptr<LocalDesktopStreamParameters> parameters,
                       std::unique_ptr<LocalScreenStreamObserver> observer);
#endif
#ifdef OWT_ENABLE_QUIC
  explicit LocalStream(std::shared_ptr<WritableStream> writer);
#endif
 private:
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  bool encoded_ = false;
#endif
#ifdef OWT_ENABLE_QUIC
  std::shared_ptr<WritableStream> writer_;
#endif
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
  explicit RemoteStream(
      const std::string& id,
      const std::string& from,
      const owt::base::SubscriptionCapabilities& subscription_capabilities,
      const owt::base::PublicationSettings& publication_settings);
  explicit RemoteStream(MediaStreamInterface* media_stream,
                        const std::string& from);
#ifdef OWT_ENABLE_QUIC
  explicit RemoteStream(std::shared_ptr<owt::base::ReadableStream> quic_stream,
                        const std::string& from);
#endif
  virtual void Attributes(
      const std::unordered_map<std::string, std::string>& attributes) {
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
      const owt::base::SubscriptionCapabilities& subscription_capabilities) {
    subscription_capabilities_ = subscription_capabilities;
  }
  /// Setter for publication settings
  void Settings(const PublicationSettings& publication_settings) {
    publication_settings_ = publication_settings;
  }
  /** @endcond */
  void Stop() {}

 protected:
  MediaStreamInterface* MediaStream();
  void MediaStream(MediaStreamInterface* media_stream);

 private:
  std::string origin_;
  bool has_audio_ = true;
  bool has_video_ = true;
#ifdef OWT_ENABLE_QUIC
  std::shared_ptr<ReadableStream> reader_;
#endif
  owt::base::SubscriptionCapabilities subscription_capabilities_;
  owt::base::PublicationSettings publication_settings_;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_STREAM_H_
