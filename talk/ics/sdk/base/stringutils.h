/*
 * Intel License
 */

#ifndef ICS_BASE_STRINGUTILS_H_
#define ICS_BASE_STRINGUTILS_H_

#include <string>

namespace ics {
namespace base {
/// This class provides utilities for string processing.
class StringUtils {
 public:
  /// Check if all characters are base 64 allowed or '='.
  static bool IsBase64EncodedString(const std::string str);
};
}
}
#endif  // ICS_BASE_STRINGUTILS_H_
