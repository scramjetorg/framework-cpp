#ifndef IFCA_STATE_H
#define IFCA_STATE_H

#include <future>

#include "ifca/drain.hpp"

namespace ifca {

struct IfcaState {
  IfcaState(unsigned int max_parallel);
  // IfcaState(const IfcaState& other) = delete;
  // IfcaState& operator=(const IfcaState& other) = delete;

  IfcaState(IfcaState&& other);
  IfcaState& operator=(IfcaState&& other);

  DrainState drain_state_;
  std::future<void> last_processing_future_;
  bool ended_;
};
}  // namespace ifca

#endif  // IFCA_STATE_H