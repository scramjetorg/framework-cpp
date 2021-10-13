#ifndef DRAIN_STATE_H
#define DRAIN_STATE_H

#include <atomic>
#include <future>
#include <mutex>

#include "types.hpp"

namespace ifca {

class DrainState {
 public:
  explicit DrainState(unsigned int max_parallel);
  DrainState(const DrainState&) = delete;
  DrainState(DrainState&&) = delete;
  DrainState& operator=(const DrainState&) = delete;
  DrainState& operator=(DrainState&&) = delete;
  ~DrainState() = default;

  operator drain_sfuture();

  void ChunkStartedProcessing();
  void ChunkFinishedProcessing();
  void ChunkReadReady();
  void ChunkRead();

 protected:
  void CheckDrain();
  bool IsDrainNeeded();
  void DrainNeeded();
  void SetDrained();

 private:
  const unsigned int max_parallel_;

  std::mutex drained_mutex;
  drain_promise drained_promise_;
  drain_sfuture drained_sfuture_;

  std::atomic_uint processing_chunks_count_;
  std::atomic_int read_chunks_count_;
};

}  // namespace ifca

#endif  // DRAIN_STATE_H
