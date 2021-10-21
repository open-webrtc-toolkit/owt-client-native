// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "postprocessing.h"

#include <cstring>

#include "backgroundblur.h"

owt::base::VideoFramePostProcessing* CreatePostProcessor(const char* name) {
  if (strcmp(name, "background_blur") == 0) {
    return new owt::ic::BackgroundBlur;
  } else {
    return nullptr;
  }
}
