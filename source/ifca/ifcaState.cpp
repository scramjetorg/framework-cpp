#include "ifca/ifcaState.hpp"

#include "helpers/Logger/logger.hpp"

namespace ifca {

IfcaState::IfcaState(unsigned int max_parallel)
    : drain_state_(max_parallel), ended_(false) {
  std::promise<void> first_processing_promise;
  first_processing_promise.set_value();
  last_processing_future_ = first_processing_promise.get_future();
  LOG_DEBUG() << "IfcaState constructor";
}

IfcaState::IfcaState(IfcaState&& other)
    : drain_state_(std::move(other.drain_state_)),
      last_processing_future_(std::move(other.last_processing_future_)),
      ended_(other.ended_) {
  LOG_DEBUG() << "IfcaState move";
}

IfcaState& IfcaState::operator=(IfcaState&& other) {
  drain_state_ = std::move(other.drain_state_);
  last_processing_future_ = std::move(other.last_processing_future_);
  ended_ = other.ended_;
  LOG_DEBUG() << "IfcaState move operator";
  return *this;
}

}  // namespace ifca