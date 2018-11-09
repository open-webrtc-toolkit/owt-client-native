/* ////////////////////////////////////////////////////////////////////////////// */
/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2005-2015 Intel Corporation. All Rights Reserved.
//
//
*/
#ifndef __SAMPLE_DEFS_H__
#define __SAMPLE_DEFS_H__
#include <memory.h>
#include "mfxdefs.h"
#include "vm/strings_defs.h"
#include "vm/file_defs.h"
#include "vm/time_defs.h"
#if defined(WIN32) || defined(WIN64)
enum {
    MFX_HANDLE_GFXS3DCONTROL = 0x100, /* A handle to the IGFXS3DControl instance */
    MFX_HANDLE_DEVICEWINDOW  = 0x101 /* A handle to the render window */
}; //mfxHandleType
#ifndef D3D_SURFACES_SUPPORT
#define D3D_SURFACES_SUPPORT 1
#endif
#if defined(_WIN32) && !defined(MFX_D3D11_SUPPORT)
#include <sdkddkver.h>
#if (NTDDI_VERSION >= NTDDI_VERSION_FROM_WIN32_WINNT2(0x0602)) // >= _WIN32_WINNT_WIN8
    #define MFX_D3D11_SUPPORT 1 // Enable D3D11 support if SDK allows
#else
    #define MFX_D3D11_SUPPORT 0
#endif
#endif // #if defined(WIN32) && !defined(MFX_D3D11_SUPPORT)
#endif // #if defined(WIN32) || defined(WIN64)
//affects win32 winnt version macro
#include "vm/time_defs.h"
#define MSDK_DEC_WAIT_INTERVAL 300000
#define MSDK_ENC_WAIT_INTERVAL 300000
#define MSDK_VPP_WAIT_INTERVAL 300000
#define MSDK_WAIT_INTERVAL MSDK_DEC_WAIT_INTERVAL+3*MSDK_VPP_WAIT_INTERVAL+MSDK_ENC_WAIT_INTERVAL // an estimate for the longest pipeline we have in samples
#define MSDK_INVALID_SURF_IDX 0xFFFF
#define MSDK_MAX_FILENAME_LEN 1024
#define MSDK_PRINT_RET_MSG(ERR) {msdk_printf(MSDK_STRING("\nReturn on error: error code %d,\t%s\t%d\n\n"), (int)ERR, MSDK_STRING(__FILE__), __LINE__);}
#define MSDK_TRACE_LEVEL(level, ERR) if (level <= msdk_trace_get_level()) {msdk_err<<NoFullPath(MSDK_STRING(__FILE__)) << MSDK_STRING(" :")<< __LINE__ <<MSDK_STRING(" [") \
    <<level<<MSDK_STRING("] ") << ERR << std::endl;}
#define MSDK_TRACE_CRITICAL(ERR) MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_CRITICAL, ERR)
#define MSDK_TRACE_ERROR(ERR) MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_ERROR, ERR)
#define MSDK_TRACE_WARNING(ERR) MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_WARNING, ERR)
#define MSDK_TRACE_INFO(ERR) MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_INFO, ERR)
#define MSDK_TRACE_DEBUG(ERR) MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_DEBUG, ERR)
#define MSDK_CHECK_ERROR(P, X, ERR)              {if ((X) == (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_CHECK_NOT_EQUAL(P, X, ERR)          {if ((X) != (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_CHECK_RESULT(P, X, ERR)             {if ((X) > (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_CHECK_PARSE_RESULT(P, X, ERR)       {if ((X) > (P)) {return ERR;}}
#define MSDK_CHECK_RESULT_SAFE(P, X, ERR, ADD)   {if ((X) > (P)) {ADD; MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_IGNORE_MFX_STS(P, X)                {if ((X) == (P)) {P = MFX_ERR_NONE;}}
#define MSDK_CHECK_POINTER(P, ...)               {if (!(P)) {return __VA_ARGS__;}}
#define MSDK_CHECK_POINTER_NO_RET(P)             {if (!(P)) {return;}}
#define MSDK_CHECK_POINTER_SAFE(P, ERR, ADD)     {if (!(P)) {ADD; return ERR;}}
#define MSDK_BREAK_ON_ERROR(P)                   {if (MFX_ERR_NONE != (P)) break;}
#define MSDK_SAFE_DELETE_ARRAY(P)                {if (P) {delete[] P; P = NULL;}}
#define MSDK_SAFE_RELEASE(X)                     {if (X) { X->Release(); X = NULL; }}
#define MSDK_SAFE_FREE(X)                        {if (X) { free(X); X = NULL; }}
#ifndef MSDK_SAFE_DELETE
#define MSDK_SAFE_DELETE(P)                      {if (P) {delete P; P = NULL;}}
#endif // MSDK_SAFE_DELETE
#define MSDK_ZERO_MEMORY(VAR)                    {memset(&VAR, 0, sizeof(VAR));}
#define MSDK_MAX(A, B)                           (((A) > (B)) ? (A) : (B))
#define MSDK_MIN(A, B)                           (((A) < (B)) ? (A) : (B))
#define MSDK_ALIGN16(value)                      (((value + 15) >> 4) << 4) // round up to a multiple of 16
#define MSDK_ALIGN32(value)                      (((value + 31) >> 5) << 5) // round up to a multiple of 32
#define MSDK_ALIGN(value, alignment)             (alignment) * ( (value) / (alignment) + (((value) % (alignment)) ? 1 : 0))
#define MSDK_ARRAY_LEN(value)                    (sizeof(value) / sizeof(value[0]))
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(par) (par)
#endif
#ifndef MFX_PRODUCT_VERSION
#define MFX_PRODUCT_VERSION "1.0.0.0"
#endif
#define MSDK_SAMPLE_VERSION MSDK_STRING(MFX_PRODUCT_VERSION)
#define MFX_IMPL_VIA_MASK(x) (0x0f00 & (x))
#endif //__SAMPLE_DEFS_H__
