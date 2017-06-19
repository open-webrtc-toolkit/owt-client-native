/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WOOGEEN_CONFERENCE_CONFERENCEEXCEPTION_H_
#define WOOGEEN_CONFERENCE_CONFERENCEEXCEPTION_H_

#include "woogeen/base/exception.h"

namespace woogeen {
namespace conference {

/// This class reprensents a conference exception.
class ConferenceException : public woogeen::base::Exception {
 public:
  enum Type : int {
    kUnknown = 3001,  // TODO(jianjun): sync with other SDKs.
  };

  ConferenceException();
  ConferenceException(const Type& type);
  ConferenceException(const Type& type, const std::string& message);

  /** @cond **/
  /**
    @brief Get the type of certain exception.
    @details The type is always unknown because we haven't define error code for conference mode.
  **/
  enum Type Type() const;
  /** @endcond **/

 private:
  const enum Type type_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_CONFERENCEEXCEPTION_H_
