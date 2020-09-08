// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_LINUX_DISPLAYUTILS_H_
#define OWT_BASE_LINUX_DISPLAYUTILS_H_

#include <X11/Xlib.h>
#include <va/va.h>
#include <va/va_x11.h>

namespace owt {
namespace base {
// Singleton for VADisplay for X11
std::shared_ptr<VADisplay> GetVADisplayX11(Display* xdpy);
}
}

#endif // OWT_BASE_LINUX_DISPLAYUTILS_H_
