// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <future>
#include <thread>
#include <vector>
#include "webrtc/rtc_base/task_queue.h"
#ifndef OWT_BASE_EVENTTRIGGER_H_
#define OWT_BASE_EVENTTRIGGER_H_
namespace owt {
namespace base {
/* @brief Functions for event execution
 * @details This class provide several static functions to execute event on its
 * observer asynchronously.
 */
class EventTrigger final {
 public:
  template <typename O, typename A, typename F>
  static void OnEvent0(std::vector<O, A> const& observers,
                       std::shared_ptr<rtc::TaskQueue> queue,
                       F func) {
    for (auto it = observers.begin(); it != observers.end(); ++it) {
      auto f = std::bind(func, *it);
      queue->PostTask([f] { f(); });
    }
  }
  template <typename O, typename A, typename F, typename T1>
  static void OnEvent1(std::vector<O, A> const& observers,
                       std::shared_ptr<rtc::TaskQueue> queue,
                       F func,
                       T1 arg1) {
    for (auto it = observers.begin(); it != observers.end(); ++it) {
      auto f = std::bind(func, *it, arg1);
      queue->PostTask([f] { f(); });
    }
  }
  template <typename O, typename A, typename F, typename T1, typename T2>
  static void OnEvent2(std::vector<O, A> const& observers,
                       std::shared_ptr<rtc::TaskQueue> queue,
                       F func,
                       T1 arg1,
                       T2 arg2) {
    for (auto it = observers.begin(); it != observers.end(); ++it) {
      auto f = std::bind(func, *it, arg1, arg2);
      queue->PostTask([f] { f(); });
    }
  }
};
}
}
#endif  // OWT_BASE_EVENTTRIGGER_H_
