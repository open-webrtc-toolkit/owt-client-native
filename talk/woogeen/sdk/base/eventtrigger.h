/*
 * Intel License
 */

#include <future>
#include <thread>
#include <vector>

#ifndef WOOGEEN_BASE_EVENTTRIGGER_H_
#define WOOGEEN_BASE_EVENTTRIGGER_H_

namespace woogeen {
namespace base {

/* @brief Functions for event execution
 * @details This class provide several static functions to execute event on its
 * observer asynchronously.
 */
class EventTrigger final {
 public:
  template <typename O, typename A, typename F, typename T1>
  static void OnEvent1(std::vector<O, A> const& observers, F func, T1 arg1) {
    for (auto it = observers.begin(); it != observers.end(); ++it) {
      auto f = std::bind(func, *it, arg1);
      // TODO(jianjunz): std::async doesn't work well on iOS. Investigate this
      // issue or use thread pool instead.
      std::thread t(f);
      t.detach();
    }
  }
  template <typename O, typename A, typename F, typename T1, typename T2>
  static void OnEvent2(std::vector<O, A> const& observers,
                       F func,
                       T1 arg1,
                       T2 arg2) {
    for (auto it = observers.begin(); it != observers.end(); ++it) {
      auto f = std::bind(func, *it, arg1, arg2);
      std::thread t(f);
      t.detach();
    }
  }
};
}
}

#endif  // WOOGEEN_BASE_EVENTTRIGGER_H_
