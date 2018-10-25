/*
 * Intel License
 */

#ifndef OMS_BASE_MEDIACONSTRAINTSIMPL_H_
#define OMS_BASE_MEDIACONSTRAINTSIMPL_H_

#include "webrtc/api/mediaconstraintsinterface.h"
#include "webrtc/rtc_base/stringencode.h"

namespace oms {
namespace base {

class MediaConstraintsImpl : public webrtc::MediaConstraintsInterface {
 public:
  MediaConstraintsImpl() {}
  virtual ~MediaConstraintsImpl() {}

  virtual const Constraints& GetMandatory() const { return mandatory_; }

  virtual const Constraints& GetOptional() const { return optional_; }

  template <typename T>
  void AddMandatory(const std::string& key, const T& value) {
    mandatory_.push_back(Constraint(key, rtc::ToString(value)));
  }

  template <typename T>
  void SetMandatory(const std::string& key, const T& value) {
    std::string value_str;
    if (mandatory_.FindFirst(key, &value_str)) {
      for (Constraints::iterator iter = mandatory_.begin();
           iter != mandatory_.end(); ++iter) {
        if (iter->key == key) {
          mandatory_.erase(iter);
          break;
        }
      }
    }
    mandatory_.push_back(Constraint(key, rtc::ToString(value)));
  }

  template <typename T>
  void AddOptional(const std::string& key, const T& value) {
    optional_.push_back(Constraint(key, rtc::ToString(value)));
  }

 private:
  Constraints mandatory_;
  Constraints optional_;
};
}
}

#endif  // OMS_BASE_MEDIACONSTRAINTSIMPL_H_
