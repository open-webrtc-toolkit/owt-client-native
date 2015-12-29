/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/p2p/p2pexception.h"

namespace woogeen {
namespace p2p {

P2PException::P2PException() : P2PException(kUnkown) {}

P2PException::P2PException(const enum Type& type)
    : P2PException(type, "Unkown P2P exception.") {}

P2PException::P2PException(const enum Type& type, const std::string& message)
    : Exception(message), type_(type) {}
}
}
