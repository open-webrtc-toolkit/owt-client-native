// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_EXCEPTION_H_
#define OWT_BASE_EXCEPTION_H_
#include <string>
namespace owt {
namespace base{
// TODO: The following exceptions need to sync with other SDKs
enum class ExceptionType : int {
  kUnknown = 1000, // General exception
  // kLocal* for local stream exceptions
  kLocalUnknown = 1100,
  kLocalDeviceNotFound = 1102,
  kLocalInvalidOption = 1104,
  kLocalNotSupported = 1105,
  // kP2P* for p2p exceptions
  kP2PUnknown = 2001,
  kP2PConnectionAuthFailed = 2121,
  kP2PMessageTargetUnreachable = 2201,
  kP2PClientUnsupportedMethod = 2401,
  kP2PClientInvalidArgument = 2402,
  kP2PClientInvalidState = 2403,
  kP2PClientRemoteNotAllowed = 2404,
  kP2PClientRemoteNotExisted = 2405,
  // kConference* for conference exceptions
  kConferenceUnknown = 3001,
  kConferenceInvalidUser,
  kConferenceInvalidParam,
  kConferenceNotSupported,
  kConferenceInvalidToken,
  kConferenceInvalidSession
};
/// Class for exceptions
class Exception {
 public:
  /// Default constructor for exceptions.
  Exception();
  /**
    @brief Constructor with type and message.
    @param type Exception type.
    @param message Exception message.
  */
  Exception(const ExceptionType& type, const std::string& message);
  virtual ~Exception() {}
  /**
    @brief Get exception type.
    @return Exception type.
  */
  ExceptionType Type() const;
  /**
    @brief Get exception message.
    @return Exception message.
  */
  std::string Message() const;
 private:
  const ExceptionType type_;
  const std::string message_;
};
}
}
#endif  // OWT_BASE_EXCEPTION_H_
