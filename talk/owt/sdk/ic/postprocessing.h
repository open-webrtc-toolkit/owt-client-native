// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_IC_POSTPROCESSING_H_
#define OWT_IC_POSTPROCESSING_H_

#include "../base/videoframepostprocessing.h"

#ifdef WEBRTC_WIN
#ifdef OWT_IC_IMPL
#define OWT_IC_EXPORT extern "C" __declspec(dllexport)
#else
#define OWT_IC_EXPORT extern "C" __declspec(dllimport)
#endif
#else
#define OWT_IC_EXPORT
#endif

OWT_IC_EXPORT owt::base::VideoFramePostProcessor* CreatePostProcessor(
    const char* name);

#endif  // OWT_IC_POSTPROCESSING_H_
