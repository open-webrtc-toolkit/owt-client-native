/*
 * Intel License
 */

#ifndef WOOGEEN_P2P_P2PEXCEPTION_H_
#define WOOGEEN_P2P_P2PEXCEPTION_H_

#include "talk/woogeen/sdk/base/exception.h"

namespace woogeen {

class P2PException : public Exception {
 public:
  enum Type : int {
    kUnkown = 2001,  // TODO(jianjun): sync with other SDKs.
    kConnAuthFailed = 2121,
    kMessageTargetUnreachable = 2201,
    kClientInvalidArgument = 2402,  // TODO(jianjun): sync with other SDK.
    kClientInvalidState = 2403,
  };

  P2PException();
  P2PException(Type type);
  P2PException(Type type, const std::string& message);

  enum Type Type();

 private:
  enum Type type_;
};
}

#endif  // WOOGEEN_P2P_P2PEXCEPTION_H_
