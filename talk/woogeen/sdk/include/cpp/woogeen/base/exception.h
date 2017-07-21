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

#ifndef WOOGEEN_BASE_EXCEPTION_H_
#define WOOGEEN_BASE_EXCEPTION_H_

#include <string>

namespace woogeen {
namespace base{

/// Base class for exceptions
class Exception {
 public:
  /// Default constructor for exceptions.
  Exception();
  virtual ~Exception() {}
  /**
    @brief Constructor with message.
    @param message Exception message.
  */
  Exception(const std::string& message);

  /**
    @brief Get exception message.
    @return Exception message.
  */
  std::string Message() const;

 private:
  const std::string message_;
};

class StreamException : public Exception {
 public:
  enum ExceptionType : int {
    kUnknown = 1100,  // General stream exceptions
    // kLocal* for local stream exceptions
    kLocalDeviceNotFound = 1102,
    kLocalInvalidOption = 1104,
    kLocalNotSupported = 1105,
  };

  StreamException();
  StreamException(const ExceptionType& type);
  StreamException(const ExceptionType& type, const std::string& message);

  ExceptionType Type() const;

 private:
  const enum ExceptionType type_;
};
}
}

#endif  // WOOGEEN_BASE_EXCEPTION_H_
