// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/include/cpp/owt/base/exception.h"
namespace owt {
namespace base {
Exception::Exception()
    : Exception(ExceptionType::kUnknown, "Unknown exception.") {}
Exception::Exception(const ExceptionType& type, const std::string& message)
    : type_(type), message_(message) {}
ExceptionType Exception::Type() const {
  return type_;
}
std::string Exception::Message() const {
  return message_;
}
}
}
