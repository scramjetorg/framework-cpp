
#include "ifca/ifca.hpp"

#include <functional>
#include <iostream>  //TODO: remove
#include <stdexcept>

#include "helpers/asyncForwarder.hpp"
#include "helpers/futureIsReady.hpp"
#include "ifca/exceptions.hpp"

namespace ifca {

Ifca::Ifca(unsigned int max_parallel)
    : drain_state_(max_parallel), ended_(false) {
  std::promise<void> first_processing_promise;
  first_processing_promise.set_value();
  last_processing_future_ = first_processing_promise.get_future();
}

Ifca::~Ifca() {}

drain_sfuture Ifca::Write(chunk_intype chunk) {
  if (ended_) throw WriteAfterEnd();

  if (read_ahead_promises_.empty()) {
    auto& promise = processing_promises_.emplace_back(chunk_promise());
    read_futures_.emplace_back(promise.get_future());
  } else {
    auto read_ahead_promise = std::move(read_ahead_promises_.front());
    read_ahead_promises_.pop_front();
    processing_promises_.push_back(std::move(read_ahead_promise));
  }

  if (transforms_.empty()) {
    auto result_promise = processing_promises_.take_front();
    result_promise.set_value(chunk);
    drain_state_.ChunkReadReady();
  } else {
    drain_state_.ChunkStartedProcessing();
    last_processing_future_ = std::async(
        std::launch::async,
        [this](chunk_intype&& chunk,
               std::future<void>&& previous_processing_future) {
          for (auto&& transform : transforms_) {
            chunk = transform(chunk);
          }
          previous_processing_future.wait();
          auto result_promise = processing_promises_.take_front();
          result_promise.set_value(chunk);
          drain_state_.ChunkFinishedProcessing();
        },
        async_forwarder<chunk_intype>(chunk),
        std::move(last_processing_future_));
  }

  return drain_state_;
}

chunk_future Ifca::Read() {
  drain_state_.ChunkRead();
  if (read_futures_.empty()) {
    if (ended_) {
      auto end_promise = chunk_promise();
      setReadEnd(end_promise);
      return end_promise.get_future();
    }
    auto& read_ahead_promise =
        read_ahead_promises_.emplace_back(chunk_promise());
    return read_ahead_promise.get_future();
  }
  auto read = std::move(read_futures_.front());
  read_futures_.pop_front();

  return read;
}

void Ifca::End() {
  if (ended_) throw MultipleEnd();
  ended_ = true;
  for (auto&& read_ahead : read_ahead_promises_) {
    setReadEnd(read_ahead);
  }
}

void Ifca::addTransform(std::function<chunk_outtype(chunk_intype)>& transform) {
  if (!transform) throw UncallableFunction();
  transforms_.push_back(transform);
}

unsigned int Ifca::maxParallel() {
  auto max_parallel = 2 * std::thread::hardware_concurrency();
  if (max_parallel == 0) return 2;
  return max_parallel;
}

void Ifca::setReadEnd(chunk_promise& promise) {
  try {
    throw ReadEnd();
  } catch (const ReadEnd&) {
    promise.set_exception(std::current_exception());
  }
}

}  // namespace ifca
