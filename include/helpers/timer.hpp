#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <condition_variable>

namespace test_utils {

struct Timer {
  template <class Rep, class Period>
  static void waitFor(std::chrono::duration<Rep, Period> rel_time) {
    std::mutex m;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lk(m);
    cv.wait_for(lk, rel_time);
  }
};

}  // namespace test_utils

#endif  // TIMER_H
