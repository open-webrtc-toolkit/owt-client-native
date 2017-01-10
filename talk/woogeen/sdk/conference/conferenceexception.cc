/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceexception.h"

namespace woogeen {
namespace conference {

ConferenceException::ConferenceException() : ConferenceException(kUnknown) {}

ConferenceException::ConferenceException(const enum Type& type)
    : ConferenceException(type, "Unkown P2P exception.") {}

ConferenceException::ConferenceException(const enum Type& type,
                                         const std::string& message)
    : Exception(message), type_(type) {}
}
}
