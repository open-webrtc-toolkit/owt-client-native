/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/stream.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/remotemixedstream.h"

namespace woogeen {

RemoteMixedStream::RemoteMixedStream(
    std::string& id,
    std::string& from,
    const std::vector<VideoFormat> supported_video_formats)
    : RemoteStream(id, from),
      supported_video_formats_(supported_video_formats) {}

const std::vector<VideoFormat> RemoteMixedStream::SupportedVideoFormats() {
  return supported_video_formats_;
}
};
