#include "ifca/drain.hpp"

#include <algorithm>

#include "helpers/futureIsReady.hpp"

namespace ifca {

DrainState::DrainState(unsigned int max_parallel)
    : max_parallel_(max_parallel),
      drained_promise_(new std::promise<void>()),
      drained_sfuture_(drained_promise_->get_future()),
      processing_chunks_count_(0),
      read_chunks_count_(0) {
  drained_promise_->set_value();
  drained_promise_.reset();
}

DrainState::operator drain_sfuture() { return drained_sfuture_; }

void DrainState::ChunkStartedProcessing() {
  processing_chunks_count_++;
  CheckDrain();
}

void DrainState::ChunkFinishedProcessing() {
  processing_chunks_count_--;
  read_chunks_count_++;
  CheckDrain();
}

void DrainState::ChunkReadReady() {
  read_chunks_count_++;
  CheckDrain();
}

void DrainState::ChunkRead() {
  read_chunks_count_--;
  CheckDrain();
}

void DrainState::CheckDrain() {
  if (IsDrainNeeded()) {
    std::lock_guard<std::mutex> m(drained_mutex);
    if (!drained_promise_) DrainNeeded();
  } else {
    std::lock_guard<std::mutex> m(drained_mutex);
    if (drained_promise_) SetDrained();
  }
}

bool DrainState::IsDrainNeeded() {
  return processing_chunks_count_.load() +
             std::max(read_chunks_count_.load(), 0) >=
         max_parallel_;
}

void DrainState::DrainNeeded() {
  drained_promise_ = drain_promise(new std::promise<void>());
  drained_sfuture_ = drain_sfuture(drained_promise_->get_future());
}

void DrainState::SetDrained() {
  drained_promise_->set_value();
  drained_promise_.reset();
}

}  // namespace ifca
