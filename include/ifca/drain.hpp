#ifndef DRAIN_STATE_H
#define DRAIN_STATE_H

#include <atomic>
#include <future>
#include <memory>
#include <mutex>

#include "types.hpp"

namespace ifca {

class DrainState {
 public:
  explicit DrainState(unsigned int max_parallel);
  DrainState(const DrainState&) = delete;
  DrainState(DrainState&&);
  DrainState& operator=(const DrainState&) = delete;
  DrainState& operator=(DrainState&&);
  ~DrainState() = default;

  bool drainOccured();
  bool drained();
  drain_sfuture get();
  operator drain_sfuture();

  void ChunkStartedProcessing();
  void ChunkFinishedProcessing();
  void ChunkRead();

 protected:
  void CheckDrain();
  bool LimitExceeded();
  void DrainNeeded();
  void SetDrained();

 private:
  unsigned int max_parallel_;

  std::mutex drained_mutex;
  std::unique_ptr<std::promise<void>> drained_promise_;
  drain_sfuture drained_sfuture_;

  std::atomic_uint processing_chunks_count_;
  std::atomic_int read_chunks_count_;
};

}  // namespace ifca

#endif  // DRAIN_STATE_H
