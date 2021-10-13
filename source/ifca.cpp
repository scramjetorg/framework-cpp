
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
  processing_future_ = first_processing_promise.get_future();
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
    auto async_future = std::async(
        std::launch::async,
        [this](chunk_intype&& chunk,
               std::future<void>&& previous_processing_future) {
          for (auto&& transform : transforms_) {
            chunk = transform(chunk);
          }

          previous_processing_future.wait();
          auto result_promise = processing_promises_.take_front();
          drain_state_.ChunkFinishedProcessing();
          result_promise.set_value(chunk);
        },
        async_forwarder<chunk_intype>(chunk), std::move(processing_future_));
    processing_future_ = std::move(async_future);
  }

  return drain_state_;
}

chunk_future Ifca::Read() {
  drain_state_.ChunkRead();
  if (read_futures_.empty()) {
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
    try {
      throw ReadEnd();
    } catch (const ReadEnd&) {
      read_ahead.set_exception(std::current_exception());
    }
  }
}

void Ifca::addTransform(
    std::function<chunk_outtype(chunk_intype)>&& transform) {
  transforms_.push_back(std::move(transform));
}

}  // namespace ifca
