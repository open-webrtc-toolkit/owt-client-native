// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "postprocessing.h"

#include <cstring>

#include "backgroundblur.h"

owt::base::VideoFramePostProcessor* CreatePostProcessor(const char* name) {
  if (name) {
    if (strcmp(name, "background_blur") == 0) {
      return new owt::ic::BackgroundBlur;
    }
  }
  return nullptr;
}
