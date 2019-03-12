// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_MACROS_H_
#define OWT_BASE_MACROS_H_
#if __cplusplus > 201402L
#define OWT_DEPRECATED [[deprecated]]
#else
#ifdef __GNUC__
#define OWT_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define OWT_DEPRECATED __declspec(deprecated)
#else
#pragma message("Deprecation is not implemented in this compiler.")
#define OWT_DEPRECATED
#endif
#endif
#endif  // OWT_BASE_MACROS_H_
