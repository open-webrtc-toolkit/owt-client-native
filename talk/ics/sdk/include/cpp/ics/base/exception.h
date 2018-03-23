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

#ifndef ICS_BASE_EXCEPTION_H_
#define ICS_BASE_EXCEPTION_H_

#include <string>

namespace ics {
namespace base{

// TODO: The following exceptions need to sync with other SDKs
enum class ExceptionType : int {
  kUnknown = 101, // General exception

  // kLocal* for local stream exceptions
  kLocalUnknown = 1100,
  kLocalDeviceNotFound = 1102,
  kLocalInvalidOption = 1104,
  kLocalNotSupported = 1105,

  // kP2P* for p2p exceptions
  kP2PUnknown = 2001,
  kP2PConnectionAuthFailed = 2121,
  kP2PMessageTargetUnreachable = 2201,
  kP2PClientUnsupportedMethod = 2401,
  kP2PClientInvalidArgument = 2402,
  kP2PClientInvalidState = 2403,
  kP2PClientRemoteNotAllowed = 2404,
  kP2PClientRemoteNotExisted = 2405,

  // kConference* for conference exceptions
  kConferenceUnknown = 3001,
  kConferenceInvalidUser,
  kConferenceInvalidParam,
  kConferenceNotSupported,
  kConferenceInvalidToken,
  kConferenceInvalidSession
};

/// Class for exceptions
class Exception {
 public:
  /// Default constructor for exceptions.
  Exception();
  /**
    @brief Constructor with type and message.
    @param type Exception type.
    @param message Exception message.
  */
  Exception(const ExceptionType& type, const std::string& message);

  virtual ~Exception() {}
  /**
    @brief Get exception type.
    @return Exception type.
  */
  ExceptionType Type() const;
  /**
    @brief Get exception message.
    @return Exception message.
  */
  std::string Message() const;

 private:
  const ExceptionType type_;
  const std::string message_;
};

}
}

#endif  // ICS_BASE_EXCEPTION_H_
