/*
 * Intel License
 */

#include "talk/woogeen/sdk/p2p/p2pexception.h"

namespace woogeen {

P2PException::P2PException() : P2PException(Type::kUnkown) {
}

P2PException::P2PException(enum Type type) : P2PException(type, "Unkown P2P exception.") {
}

P2PException::P2PException(enum Type type, const std::string& message)
    : Exception(message),
      type_(type) {
}
}
