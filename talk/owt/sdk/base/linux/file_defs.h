// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_LINUX_FILE_DEFS_H_
#define OWT_BASE_LINUX__FILE_DEFS_H_

#include "mfx/mfxdefs.h"

#include <stdio.h>

#include <unistd.h>

#define MSDK_FOPEN(file, name, mode) !(file = fopen(name, mode))

#define msdk_fgets  fgets

#endif // OWT_BASE_LINUX_FILE_DEFS_H_
