#ifndef FUTURE_IS_READY_H
#define FUTURE_IS_READY_H

#include <future>

template <typename R>
bool future_is_ready(std::future<R> const& f) {
  return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template <typename R>
bool future_is_ready(std::shared_future<R> const& f) {
  return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

#endif  // FUTURE_IS_READY_H
