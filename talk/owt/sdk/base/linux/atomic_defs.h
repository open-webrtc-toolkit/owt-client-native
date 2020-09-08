// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __ATOMIC_DEFS_H__
#define __ATOMIC_DEFS_H__

#include "mfx/mfxdefs.h"

/* Thread-safe 16-bit variable incrementing */
mfxU16 msdk_atomic_inc16(volatile mfxU16 *pVariable);

/* Thread-safe 16-bit variable decrementing */
mfxU16 msdk_atomic_dec16(volatile mfxU16 *pVariable);

/* Thread-safe 32-bit variable incrementing */
mfxU32 msdk_atomic_inc32(volatile mfxU32 *pVariable);

/* Thread-safe 32-bit variable decrementing */
mfxU32 msdk_atomic_dec32(volatile mfxU32 *pVariable);

#endif // #ifndef __ATOMIC_DEFS_H__