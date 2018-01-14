/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/base/exception.h"

namespace ics {
namespace base {

Exception::Exception() : Exception("Unknown exception.") {}

Exception::Exception(const std::string& message) : message_(message) {}

std::string Exception::Message() const {
  return message_;
}

StreamException::StreamException() : StreamException(kUnknown) {}

StreamException::StreamException(const enum ExceptionType& type)
    : StreamException(type, "Unkown stream exception.") {}

StreamException::StreamException(const enum ExceptionType& type,
                                 const std::string& message)
    : Exception(message), type_(type) {}

StreamException::ExceptionType StreamException::Type() const {
  return type_;
}

}
}
