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
#include "localcamerastreamparameters.h"

namespace webrtc {
  class MediaStreamInterface;
}

namespace cricket {
  class RawFramesCapturer;
}

class FrameGeneratorInterface;

namespace woogeen {

class MediaConstraintsImpl;

using webrtc::MediaStreamInterface;

class Stream {
  friend class ConferencePeerConnectionChannel;

 public:
  MediaStreamInterface* MediaStream() const;
  const std::string& Id() const;
  virtual void DisableAudio();
  virtual void DisableVideo();
  virtual void EnableAudio();
  virtual void EnableVideo();

 protected:
  Stream(std::string& id);
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

class LocalStream : public Stream {
 public:
  LocalStream();
  virtual ~LocalStream();
 protected:
  woogeen::MediaConstraintsImpl* media_constraints_;
};

class RemoteStream : public Stream {
  friend class ConferencePeerConnectionChannel;

 public:
  // Return the remote user ID, indicates who published this stream.
  std::string& From();

 protected:
  explicit RemoteStream(std::string& id, std::string& from);
  explicit RemoteStream(MediaStreamInterface* media_stream, std::string& from);

  MediaStreamInterface* MediaStream();
  void MediaStream(MediaStreamInterface* media_stream);

 private:
  std::string& remote_user_id_;
};

class RemoteCameraStream : public RemoteStream {
 public:
  explicit RemoteCameraStream(std::string& id, std::string& from);
  explicit RemoteCameraStream(MediaStreamInterface* media_stream,
                              std::string& from);
};

class RemoteScreenStream : public RemoteStream {
 public:
  explicit RemoteScreenStream(std::string& id, std::string& from);
  explicit RemoteScreenStream(MediaStreamInterface* media_stream,
                              std::string& from);
};

class LocalCameraStream : public LocalStream {
 public:
  explicit LocalCameraStream(
      std::shared_ptr<LocalCameraStreamParameters> parameters);
  ~LocalCameraStream();
};

class LocalRawStream : public LocalStream {
  public:
  explicit LocalRawStream(std::shared_ptr<LocalCameraStreamParameters> parameters, FrameGeneratorInterface* framer);
  ~LocalRawStream();

  private:
   cricket::RawFramesCapturer* capturer_;
};

}

#endif  // WOOGEEN_BASE_STREAM_H_
