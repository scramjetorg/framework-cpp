#ifndef IFCA_METHODS_H
#define IFCA_METHODS_H

#include <thread>

#include "helpers/Logger/logger.hpp"
#include "helpers/asyncForwarder.hpp"
#include "helpers/threadList.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/ifcaState.hpp"
#include "ifca/types.hpp"

namespace ifca {

/**
 * @brief Ifca base class with base methods implementation
 *
 * IfcaMethods is a base ifca class attached with CRTP
 *
 * @tparam Impl typename used to cast on derived methods
 * @tparam input_type input type used by ifca algorithm
 * @tparam output_type output type used by ifca algorithm
 */
template <typename Impl, typename input_type, typename output_type>
class IfcaMethods {
 public:
  // using exact_type = Impl;
  using chunk_promise = typename std::promise<output_type>;
  using chunk_future = typename std::future<output_type>;

  IfcaMethods(unsigned int max_parallel) : state_(max_parallel){};
  IfcaMethods(IfcaState&& state) : state_(std::move(state)){};

  // IfcaMethods(IfcaMethods&& other)
  //     : state_(std::move(other.state_)),
  //       processing_promises_(std::move(other.processing_promises_)),
  //       read_ahead_promises_(std::move(other.read_ahead_promises_)),
  //       read_futures_(std::move(other.read_futures_)){};

  // IfcaMethods& operator=(IfcaMethods&& other) {
  //   state_ = std::move(other.state_);
  //   processing_promises_ = std::move(other.processing_promises_);
  //   read_ahead_promises_ = std::move(other.read_ahead_promises_);
  //   read_futures_ = std::move(other.read_futures_);
  //   return *this;
  // };

  Impl& derived() { return static_cast<Impl&>(*this); }
  Impl const& derived() const { return static_cast<Impl const&>(*this); }

  drain_sfuture write(input_type chunk) {
    if (state_.ended_) throw WriteAfterEnd();

    // TODO: check how much is processing then create
    if (read_ahead_promises_.empty()) {
      auto& promise = processing_promises_.emplace_back(chunk_promise());
      read_futures_.emplace_back(promise.get_future());
    } else {
      auto read_ahead_promise = std::move(read_ahead_promises_.front());
      read_ahead_promises_.pop_front();
      processing_promises_.push_back(std::move(read_ahead_promise));
    }

    state_.drain_state_.ChunkStartedProcessing();
    state_.last_processing_future_ = std::async(
        std::launch::async,
        [this](input_type&& chunk,
               std::future<void>&& previous_processing_future) {
          try {
            auto&& transforms_result = derived()(FWD(chunk));
            previous_processing_future.wait();
            auto result_promise = processing_promises_.take_front();
            result_promise.set_value(FWD(transforms_result));
          } catch (std::invalid_argument& e) {
            printf("CATCHED\n");
          }
          state_.drain_state_.ChunkFinishedProcessing();
        },
        async_forwarder<input_type>(chunk),
        std::move(state_.last_processing_future_));

    return state_.drain_state_;
  }

  chunk_future read() {
    state_.drain_state_.ChunkRead();
    if (read_futures_.empty()) {
      if (state_.ended_) {
        auto end_promise = chunk_promise();
        setReadEnd(end_promise);
        LOG_ERROR() << "4";
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
    if (state_.ended_) throw MultipleEnd();
    state_.ended_ = true;
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

  IfcaState state_;

 private:
  ThreadList<chunk_promise> processing_promises_;
  std::list<chunk_promise> read_ahead_promises_;
  std::list<chunk_future> read_futures_;
};

}  // namespace ifca

#endif  // IFCA_METHODS_H