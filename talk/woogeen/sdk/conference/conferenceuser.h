/*
 * Intel License
 */

#ifndef WOOGEEN_CONFERENCE_USER_H_
#define WOOGEEN_CONFERENCE_USER_H_

#include <string>

namespace woogeen {
namespace conference {
// This class represent a user's permission.
class Permission {
 public:
  Permission(bool publish, bool subscribe, bool record)
      : publish_(publish), record_(record), subscribe_(subscribe) {}

  // Indicates whether publish is allowed.
  bool CanPublish() const { return publish_; };
  // Indicates whether record is allowed.
  bool CanRecord() const { return record_; };
  // Indicates whether subscribe is allowed.
  bool CanSubscribe() const { return subscribe_; };

 private:
  bool publish_;
  bool record_;
  bool subscribe_;
};

// This class represent an attendee of a conference
class User {
 public:
  User(std::string id,
       std::string name,
       std::string role,
       Permission permissions)
      : role_(role), id_(id), permissions_(permissions) {}
  std::string Role() const { return role_; };
  std::string Name() const { return name_; };
  std::string Id() const { return id_; };
  Permission Permissions() const { return permissions_; };

 private:
  std::string role_;
  std::string name_;
  std::string id_;
  class Permission permissions_;
};

}  // conference
}  // woogeen

#endif  // WOOGEEN_CONFERENCE_USER_H_
