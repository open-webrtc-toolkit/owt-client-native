/******************************************************************************\
Copyright (c) 2005-2018, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <fcntl.h>
#include <unistd.h>
#include <shared_mutex>
#include <string.h>
#include <mutex>
#include <sys/stat.h>

#include "webrtc/rtc_base/logging.h"
#include "displayutils.h"

namespace owt {
namespace base {

struct VADisplayX11Terminator {
    VADisplayX11Terminator() {}
    void operator()(VADisplay* display)
    {
        vaTerminate(*display);
        delete display;
    }
};

class DisplayGetter {
 public:
  static std::shared_ptr<VADisplay> GetVADisplayX11(Display* xdpy) {
    std::shared_lock<std::shared_timed_mutex> lock(mutex_x11_);
    if (display_x11_)
      return display_x11_;

    if (!xdpy) {
      RTC_LOG(LS_ERROR) << "Empty X11 display detected.";
      return display_x11_;
    }

    VADisplay vaDisplay = vaGetDisplay(xdpy);
    int majorVersion, minorVersion;
    VAStatus status = vaInitialize(vaDisplay, &majorVersion, &minorVersion);
    if (status != VA_STATUS_SUCCESS) {
      RTC_LOG(LS_ERROR) << "Failed to vaInitialize for X11 rendering.";
      return display_x11_;
    }
    display_x11_.reset(new VADisplay(vaDisplay), VADisplayX11Terminator());
    return display_x11_;
  }

 private:
  static std::shared_timed_mutex mutex_x11_;
  static std::shared_ptr<VADisplay> display_x11_;
};

std::shared_timed_mutex DisplayGetter::mutex_x11_;
std::shared_ptr<VADisplay> DisplayGetter::display_x11_;

std::shared_ptr<VADisplay> GetVADisplayX11(Display* xdpy) {
  return DisplayGetter::GetVADisplayX11(xdpy);
}

}
}
