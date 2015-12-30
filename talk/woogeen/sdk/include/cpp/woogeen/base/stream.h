/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
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

#include <memory>
#include "woogeen/base/localcamerastreamparameters.h"
#include "woogeen/base/videorendererinterface.h"

namespace webrtc {
  class MediaStreamInterface;
}

namespace cricket {
  class RawFramesCapturer;
}

class FrameGeneratorInterface;

namespace woogeen {

namespace conference{
  class ConferencePeerConnectionChannel;
}

namespace base {

class MediaConstraintsImpl;

using webrtc::MediaStreamInterface;

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
  /// Attach the stream to a renderer, so it can be displayed.
  virtual void Attach(VideoRendererRGBInterface& renderer);

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
};

/**
  @brief This class represent a local stream.
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

 public:
  /// Return the remote user ID, indicates who published this stream.
  std::string From();

 protected:
  explicit RemoteStream(std::string& id, std::string& from);
  explicit RemoteStream(MediaStreamInterface* media_stream, std::string& from);

  MediaStreamInterface* MediaStream();
  void MediaStream(MediaStreamInterface* media_stream);

 private:
  std::string& remote_user_id_;
};

/// This class represent a remote stream captured from a camera and/or mic.
class RemoteCameraStream : public RemoteStream {
 public:
  /** @cond */
  explicit RemoteCameraStream(std::string& id, std::string& from);
  explicit RemoteCameraStream(MediaStreamInterface* media_stream,
                              std::string& from);
  /** @endcond */
};

/// This class represent a remote stream captured from screen sharing.
class RemoteScreenStream : public RemoteStream {
 public:
  /** @cond */
  explicit RemoteScreenStream(std::string& id, std::string& from);
  explicit RemoteScreenStream(MediaStreamInterface* media_stream,
                              std::string& from);
  /** @endcond */
};

/// This class represent a local stream captured from camera, mic.
class LocalCameraStream : public LocalStream {
 public:
  /**
    Initialize a LocalCameraStream with parameters.
    @param parameters Parameters for creating the stream. The stream will not be
    impacted if chaning parameters after it is created.
  */
  explicit LocalCameraStream(const LocalCameraStreamParameters& parameters);
  ~LocalCameraStream();
};

/// This class represent a local stream which use frame generator to generate frames.
class LocalRawStream : public LocalStream {
  public:
  /**
    Initialize a LocalRawStream with parameters.
    @param parameters Parameters for creating the stream. The stream will not be
    impacted if chaning parameters after it is created.
    @param framer An instance implemented FrameGeneratorInterface.
  */
   explicit LocalRawStream(
       std::shared_ptr<LocalCameraStreamParameters> parameters,
       FrameGeneratorInterface* framer);
   ~LocalRawStream();

  private:
   cricket::RawFramesCapturer* capturer_;
};
}
}

#endif  // WOOGEEN_BASE_STREAM_H_
