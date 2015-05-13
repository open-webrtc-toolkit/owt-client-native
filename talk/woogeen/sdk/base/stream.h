/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_STREAM_H_
#define WOOGEEN_BASE_STREAM_H_

#include "talk/app/webrtc/mediastreaminterface.h"

namespace woogeen {

using webrtc::MediaStreamInterface;
using rtc::scoped_refptr;

class Stream : public rtc::RefCountInterface {
  public:
    scoped_refptr<MediaStreamInterface> MediaStream();

  protected:
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);
    scoped_refptr<MediaStreamInterface> media_stream_;
};

class LocalStream : public Stream {
};

class RemoteStream : public Stream {
};

class LocalCameraStream : public LocalStream {
  public:
    static scoped_refptr<LocalCameraStream> Create(cricket::VideoCapturer* capturer);

  protected:
    explicit LocalCameraStream(cricket::VideoCapturer* capturer);
    ~LocalCameraStream() {
      LOG(LS_INFO) << "~LocalCameraStream";
    }
};
}

#endif  // WOOGEEN_BASE_STREAM_H_
