/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/p2p/p2pexception.h"

namespace ics {
namespace p2p {

P2PException::P2PException() : P2PException(kUnknown) {}

P2PException::P2PException(const enum ExceptionType& type)
    : P2PException(type, "Unkown P2P exception.") {}

P2PException::P2PException(const enum ExceptionType& type,
                           const std::string& message)
    : Exception(message), type_(type) {}

P2PException::ExceptionType P2PException::Type() {
  return type_;
}

}
}
