/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/stream.h"
#include "talk/woogeen/sdk/conference/remotemixedstream.h"

namespace woogeen {

RemoteMixedStream::RemoteMixedStream(std::string& id, std::string& from)
    : RemoteStream(id, from) {}
}
