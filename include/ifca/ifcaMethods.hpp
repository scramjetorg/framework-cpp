#ifndef IFCA_METHODS_H
#define IFCA_METHODS_H

#include "ifca/exceptions.hpp"

namespace ifca {

#include <thread>

inline unsigned int maxParallel() {
  auto max_parallel = 2 * std::thread::hardware_concurrency();
  if (max_parallel == 0) return 2;
  return max_parallel;
}

template <typename IfcaResultType>
void end(bool& ended,
         std::list<std::promise<IfcaResultType>>& read_ahead_promises) {
  if (ended) throw MultipleEnd();
  ended = true;
  for (auto&& read_ahead : read_ahead_promises) {
    setReadEnd(read_ahead);
  }
}

template <typename IfcaResultType>
inline void setReadEnd(IfcaResultType& promise) {
  try {
    throw ReadEnd();
  } catch (const ReadEnd&) {
    promise.set_exception(std::current_exception());
  }
}

}  // namespace ifca

#endif  // IFCA_METHODS_H