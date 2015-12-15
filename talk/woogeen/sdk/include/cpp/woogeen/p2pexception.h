/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
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

#ifndef WOOGEEN_P2P_P2PEXCEPTION_H_
#define WOOGEEN_P2P_P2PEXCEPTION_H_

#include "woogeen/exception.h"

namespace woogeen {

class P2PException : public Exception {
 public:
  enum Type : int {
    kUnkown = 2001,  // TODO(jianjun): sync with other SDKs.
    kConnAuthFailed = 2121,
    kMessageTargetUnreachable = 2201,
    kClientInvalidArgument = 2402,  // TODO(jianjun): sync with other SDK.
    kClientInvalidState = 2403,
  };

  P2PException();
  P2PException(Type type);
  P2PException(Type type, const std::string& message);

  enum Type Type();

 private:
  enum Type type_;
};
}

#endif  // WOOGEEN_P2P_P2PEXCEPTION_H_
