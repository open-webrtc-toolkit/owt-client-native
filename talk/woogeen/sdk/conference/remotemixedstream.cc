/*
 * Intel License
 */

#include <algorithm>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/remotemixedstream.h"
#include "webrtc/base/logging.h"

namespace woogeen {
namespace conference {

RemoteMixedStream::RemoteMixedStream(
    const std::string& id,
    const std::string& from,
    const std::string& viewport,
    const std::vector<VideoFormat> supported_video_formats)
    : RemoteStream(id, from),
      viewport_(viewport),
      supported_video_formats_(supported_video_formats) {}

std::vector<VideoFormat> RemoteMixedStream::SupportedVideoFormats() {
  return supported_video_formats_;
}

void RemoteMixedStream::AddObserver(RemoteMixedStreamObserver& observer){
  observers_.push_back(observer);
}

void RemoteMixedStream::RemoveObserver(RemoteMixedStreamObserver& observer){
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<RemoteMixedStreamObserver> o) -> bool {
        return &observer == &(o.get());
      }));
}

std::string RemoteMixedStream::Viewport() {
  return viewport_;
}

void RemoteMixedStream::OnVideoLayoutChanged() {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnVideoLayoutChanged();
  }
}
}
};
