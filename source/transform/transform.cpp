#include "ifca/transform/transform.hpp"

#include <future>
#include <iostream>

namespace ifca {

Transform::Transform(std::function<std::string(std::string)> func)
    : func_(func) {}

void Transform::Run(ThreadList<chunk_promise>& processing_promises_,
                    std::future<void>&& previous_ready, std::string&& chunk) {
  auto res = func_(chunk);
  auto next = GetNextTransform();
  if (next) {
    // std::cout << "--------wchodzi\n";
    previous_ready.wait();
    auto result_promise = processing_promises_.take_front();
    result_promise.set_value(std::move(res));
    // next->Run(processing_promises_, std::move(previous_ready),
    // std::move(res));
  } else {
    // std::cout << "--------tuwchodzi\n";
    previous_ready.wait();
    auto result_promise = processing_promises_.take_front();
    result_promise.set_value(std::move(res));
  }
}

}  // namespace ifca