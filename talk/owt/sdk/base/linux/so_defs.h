// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __SO_DEFS_H__
#define __SO_DEFS_H__

#include "mfx/mfxdefs.h"
#include "strings_defs.h"

/* Declare shared object handle */
typedef void* msdk_so_handle;
typedef void (*msdk_func_pointer)(void);

msdk_so_handle msdk_so_load(const msdk_char* file_name);
msdk_func_pointer msdk_so_get_addr(msdk_so_handle handle,
                                   const char* func_name);
void msdk_so_free(msdk_so_handle handle);

#endif  // #ifndef __SO_DEFS_H__
