/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_REMOTEMIXEDSTREAM_H_
#define WOOGEEN_CONFERENCE_REMOTEMIXEDSTREAM_H_

#include "talk/woogeen/sdk/base/stream.h"

namespace woogeen {
class RemoteMixedStream : public RemoteStream {
  public:
    RemoteMixedStream(std::string& id);
};
}

#endif  // WOOGEEN_CONFERENCE_REMOTEMIXEDSTREAM_H_
