/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/exception.h"

namespace woogeen {

Exception::Exception() : Exception ("Unknown exception.") {
}

Exception::Exception(const std::string& message)
    : message_(message) {
}

std::string Exception::Message(){
  return message_;
}

}
