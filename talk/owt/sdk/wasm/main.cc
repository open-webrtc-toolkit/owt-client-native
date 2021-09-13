// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <emscripten/bind.h>
#include <iostream>
#include "third_party/webrtc/rtc_base/strings/json.h"
#include "talk/owt/sdk/wasm/binding.h"

int main(int argc, char** argv) {
  Json::Value in_s("foo");
  std::string out;
  rtc::GetStringFromJson(in_s, &out);
  std::cout << out << std::endl;

  printf("WASM main.\n");
  return 0;
}