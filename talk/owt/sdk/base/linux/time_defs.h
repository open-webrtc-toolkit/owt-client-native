// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __TIME_DEFS_H__
#define __TIME_DEFS_H__

#include "mfx/mfxdefs.h"
#include "mfx_itt_trace.h"

#include <unistd.h>

#define MSDK_SLEEP(msec)        \
  do {                          \
    MFX_ITT_TASK("MSDK_SLEEP"); \
    usleep(1000 * msec);        \
  } while (0)

#define MSDK_USLEEP(usec)        \
  do {                           \
    MFX_ITT_TASK("MSDK_USLEEP"); \
    usleep(usec);                \
  } while (0)

#define MSDK_GET_TIME(T, S, F) ((mfxF64)((T) - (S)) / (mfxF64)(F))

typedef mfxI64 msdk_tick;

msdk_tick msdk_time_get_tick(void);
msdk_tick msdk_time_get_frequency(void);
mfxU64 rdtsc(void);

#endif  // #ifndef __TIME_DEFS_H__
