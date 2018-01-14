/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ICS_CONFERENCE_USER_H_
#define ICS_CONFERENCE_USER_H_

#include <string>

namespace ics {
namespace conference {
/// This class represent a user's permission.
class Permission {
 public:
  /** @cond */
  Permission(bool publish, bool subscribe, bool record)
      : publish_(publish), record_(record), subscribe_(subscribe) {}

  /// Indicates whether publish is allowed.
  bool CanPublish() const { return publish_; };
  /// Indicates whether record is allowed.
  bool CanRecord() const { return record_; };
  /// Indicates whether subscribe is allowed.
  bool CanSubscribe() const { return subscribe_; };
  /** @endcond */

 private:
  bool publish_;
  bool record_;
  bool subscribe_;
};

/// This class represent an attendee of a conference, replaced by Participant class
class User {
 public:
  User(std::string id,
       std::string name,
       std::string role,
       Permission permissions)
      : role_(role), name_(name), id_(id), permissions_(permissions) {}
  /// Get user's role.
  std::string Role() const { return role_; };
  /// Get user's name.
  std::string Name() const { return name_; };
  /// Get user's ID.
  std::string Id() const { return id_; };
  /** @cond */
  Permission Permissions() const { return permissions_; };
  /** @endcond */

 private:
  std::string role_;
  std::string name_;
  std::string id_;
  class Permission permissions_;
};

}  // conference
}  // ics

#endif  // ICS_CONFERENCE_USER_H_
