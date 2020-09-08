// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __STRING_DEFS_H__
#define __STRING_DEFS_H__

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <string>
#endif

#define MSDK_STRING(x) x
#define MSDK_CHAR(x) x

#ifdef __cplusplus
typedef std::string msdk_tstring;
#endif
typedef char msdk_char;

#define msdk_printf printf
#define msdk_sprintf sprintf
#define msdk_vprintf vprintf
#define msdk_fprintf fprintf
#define msdk_strlen strlen
#define msdk_strcmp strcmp
#define msdk_stricmp strcasecmp
#define msdk_strncmp strncmp
#define msdk_strstr strstr
#define msdk_atoi atoi
#define msdk_atoll atoll
#define msdk_strtol strtol
#define msdk_strtod strtod
#define msdk_itoa_decimal(value, str) \
  snprintf(str, sizeof(str) / sizeof(str[0]) - 1, "%d", value)
#define msdk_strnlen(str, maxlen) strlen(str)
#define msdk_sscanf sscanf

#define msdk_strcopy strcpy

#define msdk_strncopy_s(dst, num_dst, src, count) strncpy(dst, src, count)

#define MSDK_MEMCPY_BITSTREAM(bitstream, offset, src, count) \
  memcpy((bitstream).Data + (offset), (src), (count))

#define MSDK_MEMCPY_BUF(bufptr, offset, maxsize, src, count) \
  memcpy((bufptr) + (offset), (src), (count))

#define MSDK_MEMCPY_VAR(dstVarName, src, count) \
  memcpy(&(dstVarName), (src), (count))

#define MSDK_MEMCPY(dst, src, count) memcpy(dst, (src), (count))

#endif  //__STRING_DEFS_H__
