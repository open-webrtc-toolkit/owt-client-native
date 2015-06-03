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
    std::string& Id();

  protected:
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);
    scoped_refptr<MediaStreamInterface> media_stream_;
    std::string id_;
};

class LocalStream : public Stream {
};

class RemoteStream : public Stream {
  public:
    static std::shared_ptr<RemoteStream> Create();
    static std::shared_ptr<RemoteStream> Create(std::string& id);

    scoped_refptr<MediaStreamInterface> MediaStream();
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);

  protected:
    explicit RemoteStream(std::string& id);
};

class LocalCameraStream : public LocalStream {
  public:
    static std::shared_ptr<LocalCameraStream> Create(cricket::VideoCapturer* capturer);

  protected:
    explicit LocalCameraStream(cricket::VideoCapturer* capturer);
};
}

#endif  // WOOGEEN_BASE_STREAM_H_
