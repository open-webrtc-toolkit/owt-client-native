/*
 * Intel License
 */

#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "webrtc/base/stringencode.h"

#ifndef WOOGEEN_BASE_MEDIACONSTRAINTSIMPL_H_
#define WOOGEEN_BASE_MEDIACONSTRAINTSIMPL_H_

namespace woogeen {
class MediaConstraintsImpl : public webrtc::MediaConstraintsInterface {
  public:
    MediaConstraintsImpl() { }
    virtual ~MediaConstraintsImpl() { }

    virtual const Constraints& GetMandatory() const {
      return mandatory_;
    }

    virtual const Constraints& GetOptional() const {
      return optional_;
    }

    template<class T>
    void AddMandatory(const std::string& key, const T& value) {
      mandatory_.push_back(Constraint(key, rtc::ToString<T>(value)));
    }

    template<class T>
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
      mandatory_.push_back(Constraint(key, rtc::ToString<T>(value)));
    }

    template <class T>
    void AddOptional(const std::string& key, const T& value) {
      optional_.push_back(Constraint(key, rtc::ToString<T>(value)));
    }

  private:
    Constraints mandatory_;
    Constraints optional_;
};
}

#endif  // WOOGEEN_BASE_MEDIACONSTRAINTSIMPL_H_
