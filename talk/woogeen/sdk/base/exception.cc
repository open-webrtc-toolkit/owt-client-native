/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/base/exception.h"

namespace woogeen {
namespace base {

Exception::Exception() : Exception("Unknown exception.") {}

Exception::Exception(const std::string& message) : message_(message) {}

std::string Exception::Message() {
  return message_;
}
}
}
