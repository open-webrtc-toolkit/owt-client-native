// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"
int main(int argc, char* argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
