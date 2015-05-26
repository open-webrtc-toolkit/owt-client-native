/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_STREAM_H_
#define WOOGEEN_BASE_STREAM_H_

#include <memory>
#include "talk/app/webrtc/mediastreaminterface.h"
#include "webrtc/base/logging.h"

namespace woogeen {

using webrtc::MediaStreamInterface;
using rtc::scoped_refptr;

class Stream {
  public:
    scoped_refptr<MediaStreamInterface> MediaStream();

  protected:
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);
    scoped_refptr<MediaStreamInterface> media_stream_;
};

class LocalStream : public Stream {
};

class RemoteStream : public Stream {
  public:
    static std::shared_ptr<RemoteStream> Create();
    scoped_refptr<MediaStreamInterface> MediaStream();
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);
};

class LocalCameraStream : public LocalStream {
  public:
    static std::shared_ptr<LocalCameraStream> Create(cricket::VideoCapturer* capturer);

  protected:
    explicit LocalCameraStream(cricket::VideoCapturer* capturer);
};
}

#endif  // WOOGEEN_BASE_STREAM_H_
