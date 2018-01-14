/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/conference/conferenceexception.h"

namespace ics {
namespace conference {

ConferenceException::ConferenceException() : ConferenceException(kUnknown) {}

ConferenceException::ConferenceException(const enum Type& type)
    : ConferenceException(type, "Unkown P2P exception.") {}

ConferenceException::ConferenceException(const enum Type& type,
                                         const std::string& message)
    : Exception(message), type_(type) {}

enum ConferenceException::Type ConferenceException::Type() const {
  return type_;
}
}
}
