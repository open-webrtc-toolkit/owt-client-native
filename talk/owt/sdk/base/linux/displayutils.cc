// Copyright (C) <2018> Intel Corporation
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
