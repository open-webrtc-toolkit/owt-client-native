// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include "talk/owt/sdk/wasm/binding.h"

int main(int argc, char** argv) {
  owt::wasm::MediaSession::Get();
  return 0;
}