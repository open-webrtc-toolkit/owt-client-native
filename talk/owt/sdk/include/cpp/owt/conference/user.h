// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CONFERENCE_USER_H_
#define OWT_CONFERENCE_USER_H_
#include <string>
#include "owt/base/export.h"
namespace owt {
namespace conference {
/// This class represent a user's permission.
class OWT_EXPORT Permission {
 public:
  /** @cond */
  Permission(bool publish, bool subscribe, bool record)
      : publish_(publish), record_(record), subscribe_(subscribe) {}
  /// Indicates whether publish is allowed.
  bool CanPublish() const { return publish_; }
  /// Indicates whether record is allowed.
  bool CanRecord() const { return record_; }
  /// Indicates whether subscribe is allowed.
  bool CanSubscribe() const { return subscribe_; }
  /** @endcond */
 private:
  bool publish_;
  bool record_;
  bool subscribe_;
};
/// This class represent an attendee of a conference, replaced by Participant class
class OWT_EXPORT User {
 public:
  User(std::string id,
       std::string name,
       std::string role,
       Permission permissions)
      : role_(role), name_(name), id_(id), permissions_(permissions) {}
  /// Get user's role.
  std::string Role() const { return role_; }
  /// Get user's name.
  std::string Name() const { return name_; }
  /// Get user's ID.
  std::string Id() const { return id_; }
  /** @cond */
  Permission Permissions() const { return permissions_; }
  /** @endcond */
 private:
  std::string role_;
  std::string name_;
  std::string id_;
  class Permission permissions_;
};
}  // conference
}  // owt
#endif  // OWT_CONFERENCE_USER_H_
