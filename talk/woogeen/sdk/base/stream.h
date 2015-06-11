/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_STREAM_H_
#define WOOGEEN_BASE_STREAM_H_

#include <memory>
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/woogeen/sdk/base/mediaconstraintsimpl.h"
#include "talk/woogeen/sdk/base/localcamerastreamparameters.h"
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
  friend class ConferencePeerConnectionChannel;

  public:
    // Return the remote user ID, indicates who published this stream.
    std::string& From();

  protected:
    explicit RemoteStream(std::string& id, std::string& from);
    explicit RemoteStream(MediaStreamInterface* media_stream, std::string& from);

    scoped_refptr<MediaStreamInterface> MediaStream();
    void MediaStream(scoped_refptr<MediaStreamInterface> media_stream);

  private:
    std::string& remote_user_id_;
};

class RemoteCameraStream : public RemoteStream {
  public:
    explicit RemoteCameraStream(std::string& id, std::string& from);
    explicit RemoteCameraStream(MediaStreamInterface* media_stream, std::string& from);
};

class RemoteScreenStream : public RemoteStream {
  public:
    explicit RemoteScreenStream(std::string& id, std::string& from);
    explicit RemoteScreenStream(MediaStreamInterface* media_stream, std::string& from);
};

class LocalCameraStream : public LocalStream {
  public:
    explicit LocalCameraStream(std::shared_ptr<LocalCameraStreamParameters> parameters);
};
}

#endif  // WOOGEEN_BASE_STREAM_H_
