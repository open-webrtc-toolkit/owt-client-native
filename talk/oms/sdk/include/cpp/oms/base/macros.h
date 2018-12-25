// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_MACROS_H_
#define OMS_BASE_MACROS_H_
#if __cplusplus > 201402L
#define OMS_DEPRECATED [[deprecated]]
#else
#ifdef __GNUC__
#define OMS_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define OMS_DEPRECATED __declspec(deprecated)
#else
#pragma message("Deprecation is not implemented in this compiler.")
#define OMS_DEPRECATED
#endif
#endif
#endif  // OMS_BASE_MACROS_H_
