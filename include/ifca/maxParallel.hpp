#ifndef MAX_PARALLEL_H
#define MAX_PARALLEL_H

#include <thread>

inline unsigned int maxParallel() {
  auto max_parallel = 2 * std::thread::hardware_concurrency();
  if (max_parallel == 0) return 2;
  return max_parallel;
}

#endif  // MAX_PARALLEL_H