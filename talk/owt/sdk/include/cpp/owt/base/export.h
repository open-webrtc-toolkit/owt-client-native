// Copyright (C) <2022> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_EXPORT_H_
#define OWT_BASE_EXPORT_H_

// OWT_EXPORT is used to mark symbols as exported or imported when OWT is
// built or used as a shared library.
// When OWT is built as a static library the RTC_EXPORT macro expands to
// nothing.
#ifdef OWT_ENABLE_SYMBOL_EXPORT
#ifdef WEBRTC_WIN

#ifdef OWT_LIBRARY_IMPL
#define OWT_EXPORT __declspec(dllexport)
#else
#define OWT_EXPORT __declspec(dllimport)
#endif

#else  // WEBRTC_WIN

#if __has_attribute(visibility) && defined(OWT_LIBRARY_IMPL)
#define OWT_EXPORT __attribute__((visibility("default")))
#endif

#endif  // WEBRTC_WIN

#endif  // OWT_ENABLE_SYMBOL_EXPORT

#ifndef OWT_EXPORT
#define OWT_EXPORT
#endif

#endif  // OWT_BASE_EXPORT_H_
