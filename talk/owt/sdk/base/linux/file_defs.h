// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __FILE_DEFS_H__
#define __FILE_DEFS_H__

#include "mfx/mfxdefs.h"

#include <stdio.h>

#include <unistd.h>

#define MSDK_FOPEN(file, name, mode) !(file = fopen(name, mode))

#define msdk_fgets  fgets

#endif // #ifndef __FILE_DEFS_H__
