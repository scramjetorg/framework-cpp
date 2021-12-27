#ifndef IFCA_METHODS_H
#define IFCA_METHODS_H

#include <future>

#include "helpers/Logger/logger.hpp"
#include "helpers/asyncForwarder.hpp"
#include "helpers/threadList.hpp"
#include "ifca/drain.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/types.hpp"

namespace ifca {

/**
 * @brief Ifca base class with base methods implementation
 *
 * IfcaMethods is a base ifca class attached with CRTP
 *
 * Derived classes should provide methods:
 *
 * using input_type = ...;
 * using output_type = ...;
 *
 * void operator()(input_type&& chunk, std::future<void>&&
 * previous_processing_future)
 *
 * @tparam Impl typename used to cast on derived methods
 * @tparam input_type input type used by ifca algorithm
 * @tparam output_type output type used by ifca algorithm
 */
template <typename Impl, typename input_type, typename output_type>
class IfcaMethods {
 public:
  using chunk_promise = typename std::promise<output_type>;
  using chunk_future = typename std::future<output_type>;

  IfcaMethods(unsigned int max_parallel)
      : drain_state_(max_parallel), ended_(false) {
    std::promise<void> first_processing_promise;
    first_processing_promise.set_value();
    last_processing_future_ = first_processing_promise.get_future();
  }

  template <typename OtherIfcaMethods>
  IfcaMethods(OtherIfcaMethods&& other)
      : drain_state_(std::move(other.drain_state_)),
        last_processing_future_(std::move(other.last_processing_future_)),
        ended_(std::move(other.ended_)) {
    static_assert(!std::is_lvalue_reference_v<OtherIfcaMethods>,
                  "Only move constructor allowed");
  }

  ~IfcaMethods() {
    if (!ended_) end();
    if (!last_processing_future_.valid()) return;
    last_processing_future_.wait();
  };

  Impl& derived() { return static_cast<Impl&>(*this); }
  Impl const& derived() const { return static_cast<Impl const&>(*this); }

  drain_sfuture write(input_type chunk) {
    if (ended_) throw WriteAfterEnd();

    // TODO: check how much is processing then create
    if (read_ahead_promises_.empty()) {
      auto& promise = processing_promises_.emplace_back(chunk_promise());
      read_futures_.emplace_back(promise.get_future());
    } else {
      auto read_ahead_promise = std::move(read_ahead_promises_.front());
      read_ahead_promises_.pop_front();
      processing_promises_.push_back(std::move(read_ahead_promise));
    }

    drain_state_.ChunkStartedProcessing();
    last_processing_future_ = std::async(
        std::launch::async,
        [this](input_type&& chunk,
               std::future<void>&& previous_processing_future) {
          derived()(FWD(chunk), std::move(previous_processing_future));
        },
        async_forwarder<input_type>(chunk), std::move(last_processing_future_));

    return drain_state_;
  }

  chunk_future read() {
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

  void end() {
    if (ended_) throw MultipleEnd();
    ended_ = true;
    for (auto&& read_ahead : read_ahead_promises_) {
      setReadEnd(read_ahead);
    }
  }

 protected:
  void setReadEnd(chunk_promise& promise) {
    try {
      throw ReadEnd();
    } catch (const ReadEnd&) {
      promise.set_exception(std::current_exception());
    }
  }

  template <typename Chunk>
  void onResolve(Chunk&& chunk,
                 std::future<void>&& previous_processing_future) {
    LOG_DEBUG() << "Chunk resolved: " << chunk;
    previous_processing_future.wait();
    auto&& result_promise = processing_promises_.take_front();
    result_promise.set_value(FWD(chunk));
    drain_state_.ChunkFinishedProcessing();
  }

  void onReject() {
    LOG_DEBUG() << "Chunk rejected";
    drain_state_.ChunkFinishedProcessing();
  }

  template <typename, typename, typename>
  friend class IfcaMethods;

  DrainState drain_state_;
  std::future<void> last_processing_future_;
  bool ended_;
  ThreadList<chunk_promise> processing_promises_;

 private:
  std::list<chunk_promise> read_ahead_promises_;
  std::list<chunk_future> read_futures_;
};

}  // namespace ifca

#endif  // IFCA_METHODS_H