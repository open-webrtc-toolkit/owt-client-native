/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCEEXCEPTION_H_
#define WOOGEEN_CONFERENCE_CONFERENCEEXCEPTION_H_

#include "talk/woogeen/sdk/base/exception.h"

namespace woogeen {

class ConferenceException : public Exception{
  public:
    enum Type : int {
      kUnkown = 3001,  // TODO(jianjun): sync with other SDKs.
    };

    ConferenceException();
    ConferenceException(Type type);
    ConferenceException(Type type, const std::string& message);

    enum Type Type();

  private:
    enum Type type_;
};
}

#endif // WOOGEEN_CONFERENCE_CONFERENCEEXCEPTION_H_
