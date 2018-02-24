/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/base/exception.h"

namespace ics {
namespace base {

Exception::Exception()
    : Exception(ExceptionType::kUnknown, "Unknown exception.") {}

Exception::Exception(const ExceptionType& type, const std::string& message)
    : type_(type), message_(message) {}

ExceptionType Exception::Type() const {
  return type_;
}

std::string Exception::Message() const {
  return message_;
}

}
}
