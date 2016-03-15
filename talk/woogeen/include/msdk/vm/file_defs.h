/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __FILE_DEFS_H__
#define __FILE_DEFS_H__

#include "mfxdefs.h"

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)

#define MSDK_FOPEN(file, name, mode) _tfopen_s(&file, name, mode)

#define msdk_fgets  _fgetts
#else // #if defined(_WIN32) || defined(_WIN64)
#include <unistd.h>

#define MSDK_FOPEN(file, name, mode) file = fopen(name, mode)

#define msdk_fgets  fgets
#endif // #if defined(_WIN32) || defined(_WIN64)

#endif // #ifndef __FILE_DEFS_H__
