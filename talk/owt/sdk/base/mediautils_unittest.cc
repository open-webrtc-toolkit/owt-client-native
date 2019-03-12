// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/mediautils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"
namespace owt {
namespace base {
TEST(MediaUtilsTest, CorrectResolutionNameHd720p){
  const Resolution r(1280, 720);
  EXPECT_EQ(MediaUtils::GetResolutionName(r),"hd720p");
}
}
}
