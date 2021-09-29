#include "ifca/transform/transform.hpp"

#include <future>
#include <iostream>

namespace ifca {

Transform::Transform(std::function<std::string(std::string)>& func)
    : func_(func) {}

void Transform::Run(std::list<chunk_promise>& processing_promises_,
                    std::mutex& processing_promises_mutex,
                    const std::future<void>& previous_ready,
                    std::string&& chunk) {
  auto res = func_(chunk);
  auto next = GetNextTransform();
  if (next) {
    next->Run(processing_promises_, processing_promises_mutex, previous_ready,
              std::move(res));
  } else {
    previous_ready.wait();
    processing_promises_mutex.lock();
    auto result_promise = std::move(processing_promises_.front());
    processing_promises_.pop_front();
    processing_promises_mutex.unlock();
    result_promise.set_value(res);
  }
}

}  // namespace ifca