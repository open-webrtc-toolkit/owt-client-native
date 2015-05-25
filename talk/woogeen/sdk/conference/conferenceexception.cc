/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/conferenceexception.h"

namespace woogeen {

ConferenceException::ConferenceException() : ConferenceException(Type::kUnkown) {
}

ConferenceException::ConferenceException(enum Type type) : ConferenceException(type, "Unkown P2P exception.") {
}

ConferenceException::ConferenceException(enum Type type, const std::string& message)
    : Exception(message),
      type_(type) {
}
}
