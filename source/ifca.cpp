
#include "ifca/ifca.hpp"

#include <functional>
#include <iostream>  //TODO: remove
#include <stdexcept>

#include "helpers/asyncForwarder.hpp"
#include "helpers/futureIsReady.hpp"
#include "ifca/exceptions.hpp"

namespace ifca {

Ifca::Ifca(unsigned int max_parallel)
    : max_parallel_(max_parallel),
      drain_promise_(new std::promise<void>()),
      drain_sfuture_(drain_promise_->get_future()),
      ended_(false) {
  std::promise<void> first_processing_promise;
  first_processing_promise.set_value();
  processing_futures_.emplace_back(first_processing_promise.get_future());
  processing_futures_current = processing_futures_.begin();
  SetClearDrain();
}

Ifca::~Ifca() {}

drain_sfuture Ifca::Write(chunk_intype chunk) {
  if (ended_) throw WriteAfterEnd();

  if (read_ahead_promises_.empty()) {
    processing_promises_mutex.lock();
    auto& promise = processing_promises_.emplace_back(chunk_promise());
    processing_promises_mutex.unlock();
    read_futures_.emplace_back(promise.get_future());
  } else {
    auto read_ahead_promise = std::move(read_ahead_promises_.front());
    processing_promises_mutex.lock();
    processing_promises_.push_back(std::move(read_ahead_promise));
    processing_promises_mutex.unlock();
    read_ahead_promises_.pop_front();  // TODO: make sure no sigsegv created
  }

  if (!transform_) {
    processing_promises_mutex.lock();
    auto result_promise = std::move(processing_promises_.front());
    processing_promises_.pop_front();
    processing_promises_mutex.unlock();
    result_promise.set_value(chunk);
  } else {
    auto previous_processing_future = processing_futures_current;
    auto f = std::async(
        std::launch::async,
        [this, previous_processing_future](chunk_intype&& chunk) {
          transform_->Run(processing_promises_, processing_promises_mutex,
                          (*previous_processing_future), std::move(chunk));
          auto lock = std::lock_guard(processing_futures_mutex);
          processing_futures_.erase(previous_processing_future);
        },
        async_forwarder<chunk_intype>(chunk));

    auto lock = std::lock_guard(processing_futures_mutex);
    processing_futures_current =
        processing_futures_.emplace(processing_futures_.end(), std::move(f));
  }

  if (IsDrainLvl() && !drain_promise_) {
    CreateDrain();
  }
  return drain_sfuture_;
}

chunk_future Ifca::Read() {
  if (read_futures_.empty()) {
    auto& read_ahead_promise =
        read_ahead_promises_.emplace_back(chunk_promise());
    return read_ahead_promise.get_future();
  }
  auto read = std::move(read_futures_.front());
  read_futures_.pop_front();

  if (drain_promise_ && !IsDrainLvl()) {
    SetClearDrain();
  }
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

void Ifca::addTransform(std::unique_ptr<TransformBase> transform) {
  transform_ = std::move(transform);
}

bool Ifca::IsDrainLvl() {
  // -1 because of first_processing_promise placed in constructor
  return processing_futures_.size() - 1 + read_futures_.size() > max_parallel_;
}

void Ifca::CreateDrain() {
  drain_promise_ = drain_promise(new std::promise<void>());
  drain_sfuture_ = drain_sfuture(drain_promise_->get_future());
}

void Ifca::SetClearDrain() {
  drain_promise_->set_value();
  drain_promise_.release();
}

}  // namespace ifca
