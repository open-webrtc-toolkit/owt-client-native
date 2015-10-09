/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_EXCEPTION_H_
#define WOOGEEN_BASE_EXCEPTION_H_

#include <string>

namespace woogeen {

// Exception for base SDK
class Exception {
 public:
  Exception();
  Exception(const std::string& message);

  std::string Message();

 private:
  const std::string& message_;
};
}

#endif  // WOOGEEN_BASE_EXCEPTION_H_
