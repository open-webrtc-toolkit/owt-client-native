/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_STREAM_H_
#define WOOGEEN_BASE_STREAM_H_

#include <memory>
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/woogeen/sdk/base/mediaconstraintsimpl.h"
#include "webrtc/base/logging.h"

namespace woogeen {

using webrtc::MediaStreamInterface;
using rtc::scoped_refptr;

class Stream {
  public:
    scoped_refptr<MediaStreamInterface> MediaStream();
    std::string& Id();

  protected:
    Stream(std::string& id);
    Stream();
    virtual ~Stream(){};
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);
    scoped_refptr<MediaStreamInterface> media_stream_;

  private:
    std::string id_;
};

class LocalStream : public Stream {
  protected:
    woogeen::MediaConstraintsImpl media_constraints_;
};

class RemoteStream : public Stream {
  public:
    explicit RemoteStream(std::string& id);
    explicit RemoteStream(MediaStreamInterface* media_stream);

    scoped_refptr<MediaStreamInterface> MediaStream();
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);
};

class RemoteCameraStream : public RemoteStream {
  public:
    explicit RemoteCameraStream(std::string& id);
};

class RemoteScreenStream : public RemoteStream {
  public:
    explicit RemoteScreenStream(std::string& id);
};

class LocalCameraStream : public LocalStream {
  public:
    static std::shared_ptr<LocalCameraStream> Create(cricket::VideoCapturer* capturer);

  protected:
    explicit LocalCameraStream(cricket::VideoCapturer* capturer);
};
}

#endif  // WOOGEEN_BASE_STREAM_H_
