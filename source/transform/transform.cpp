#include "ifca/transform/transform.hpp"

#include <future>
#include <iostream>

namespace ifca {

Transform::Transform(std::function<std::string(std::string)>& func)
    : func_(func) {}

void Transform::Run(ThreadList<chunk_promise>& processing_promises_,
                    const std::future<void>& previous_ready,
                    std::string&& chunk) {
  auto res = func_(chunk);
  auto next = GetNextTransform();
  if (next) {
    next->Run(processing_promises_, previous_ready, std::move(res));
  } else {
    previous_ready.wait();
    auto result_promise = processing_promises_.take_front();
    result_promise.set_value(res);
  }
}

}  // namespace ifca