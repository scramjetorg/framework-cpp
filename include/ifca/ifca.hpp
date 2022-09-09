#ifndef IFCA_H
#define IFCA_H

#include <atomic>
#include <exception>
#include <future>
#include <list>
#include <mutex>
#include <tuple>
#include <type_traits>

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/asyncForwarder.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/maxParallel.hpp"
#include "ifca/transform/isTransformExpression.hpp"
#include "transformChain.hpp"

namespace ifca {

template <typename In, typename Out, typename... TransformChain>
class IfcaImpl {
 public:
  using input_type = In;
  using output_type = Out;

  using chunk_promise = typename std::promise<output_type>;
  using chunk_future = typename std::future<output_type>;

  explicit IfcaImpl(unsigned int max_parallel = maxParallel())
      : max_parallel(max_parallel),
        ended_(false),
        resolved_promises(0),
        read_ahead_promises(0),
        drained_promise_(std::make_unique<std::promise<void>>()),
        drained_sfuture_(drained_promise_->get_future().share()),
        processed_chunks(0),
        drained(true) {
    std::promise<void> first_processing_promise;
    first_processing_promise.set_value();
    last_processing_future_ = first_processing_promise.get_future();
    drained_promise_->set_value();
    drained_promise_.reset();
  };

  template <typename Input, typename Output, typename... Transforms,
            typename = std::enable_if_t<std::is_same_v<
                std::tuple<Transforms...>, std::tuple<TransformChain...>>>>
  explicit IfcaImpl(IfcaImpl<Input, Output, Transforms...>&& ifca)
      : transforms_(std::move(ifca.transforms_)),
        max_parallel(ifca.max_parallel),
        last_processing_future_(std::move(ifca.last_processing_future_)),
        ended_(ifca.ended_.load()),
        resolved_promises(0),
        read_ahead_promises(0),
        drained_promise_(std::make_unique<std::promise<void>>()),
        drained_sfuture_(drained_promise_->get_future().share()),
        processed_chunks(0),
        drained(true) {
    drained_promise_->set_value();
    drained_promise_.reset();
  }

  template <typename Input, typename Output, typename... Transforms,
            typename Transform>
  explicit IfcaImpl(IfcaImpl<Input, Output, Transforms...>&& ifca,
                    Transform&& transform)
      : max_parallel(ifca.max_parallel),
        transforms_(detail::ForwardTransformChain(std::move(ifca.transforms_),
                                                  FWD(transform))),
        last_processing_future_(std::move(ifca.last_processing_future_)),
        ended_(ifca.ended_.load()),
        resolved_promises(0),
        read_ahead_promises(0),
        drained_promise_(std::make_unique<std::promise<void>>()),
        drained_sfuture_(drained_promise_->get_future().share()),
        processed_chunks(0),
        drained(true) {
    drained_promise_->set_value();
    drained_promise_.reset();
  }

  IfcaImpl(const IfcaImpl&) = delete;
  IfcaImpl(IfcaImpl&&) = default;
  IfcaImpl& operator=(const IfcaImpl&) = delete;
  IfcaImpl& operator=(IfcaImpl&&) = delete;

  ~IfcaImpl() {
    if (!ended_) end();
    last_processing_future_ = std::future<void>();
    if (!last_processing_future_.valid()) return;
  };

  template <typename Input = input_type>
  std::enable_if_t<!std::is_void_v<Input>, std::shared_future<void>> write(
      Input&& chunk) {
    if (ended_) throw WriteAfterEnd();

    incrementProcessedChunks();
    last_processing_future_ = std::async(
        std::launch::async,
        [&](auto&& chunk, std::future<void> previous_processing_future) {
          if constexpr (sizeof...(TransformChain) > 0) {
            runTransforms(
                FWD(chunk), std::move(previous_processing_future),
                std::make_index_sequence<sizeof...(TransformChain)>{});
          } else {
            onResolve(FWD(chunk), std::move(previous_processing_future));
          }
        },
        FWD(chunk), std::move(last_processing_future_));

    std::lock_guard<std::mutex> m(drained_mutex);
    return drained_sfuture_;
  }

  template <typename Input = input_type>
  std::enable_if_t<!std::is_void_v<Input>, chunk_future> read() {
    if (ended_) {
      auto end_promise = chunk_promise();
      setReadEnd(end_promise);
      return end_promise.get_future();
    }

    std::lock_guard<std::mutex> m(chunk_promises_mutex);
    if (resolved_promises > read_ahead_promises) {
      auto future = chunk_promises.front().get_future();
      chunk_promises.pop_front();
      --resolved_promises;
      decrementProcessedChunks();
      return future;
    } else {
      auto promise = chunk_promise();
      auto future = promise.get_future();
      chunk_promises.push_back(std::move(promise));
      ++read_ahead_promises;
      return future;
    }
  }

  template <typename Transform, typename Enable = std::enable_if_t<
                                    is_transform_expression_v<Transform>>>
  auto addTransform(Transform&& transform) {
    if constexpr ((sizeof...(TransformChain)) > 0)
      return IfcaImpl<input_type, typename Transform::output_type,
                      TransformChain..., Transform>(std::move(*this),
                                                    FWD(transform));
    else
      return IfcaImpl<typename Transform::input_type,
                      typename Transform::output_type, Transform>(
          std::move(*this), FWD(transform));
  }

  bool ended() { return ended_; }
  void end() {
    if (ended_) throw MultipleEnd();
    ended_ = true;
    std::lock_guard<std::mutex> m(chunk_promises_mutex);
    while (read_ahead_promises > 0) {
      setReadEnd(chunk_promises.front());
      chunk_promises.pop_front();
      --read_ahead_promises;
    }
  }

  bool isDrained() {
    std::lock_guard<std::mutex> m(drained_mutex);
    return drained;
  }

  template <typename, typename, typename...>
  friend class IfcaImpl;

 protected:
  void setReadEnd(chunk_promise& promise) {
    promise.set_exception(std::make_exception_ptr(ReadEnd()));
  }

  template <typename Chunk, std::size_t... Is>
  void runTransforms(Chunk&& chunk,
                     std::future<void>&& previous_processing_future,
                     std::index_sequence<Is...>) {
    runTransformsImpl(FWD(chunk), std::move(previous_processing_future),
                      std::get<Is>(transforms_)...);
  }

  template <typename Chunk, typename FirstTransform,
            typename... RestOfTransforms>
  void runTransformsImpl(Chunk&& chunk,
                         std::future<void>&& previous_processing_future,
                         FirstTransform&& firstTransform,
                         RestOfTransforms&&... restOfTransforms) {
    auto&& onResolveCallback = [&previous_processing_future,
                                this](auto&& resolvedValue) {
      onResolve(FWD(resolvedValue), std::move(previous_processing_future));
    };
    auto&& onRejectCallback = [this] { onReject(); };
    firstTransform(FWD(chunk), FWD(onResolveCallback), FWD(onRejectCallback),
                   FWD(restOfTransforms)...);
  }

  template <typename Chunk>
  void onResolve(Chunk&& chunk,
                 std::future<void>&& previous_processing_future) {
    previous_processing_future.wait();
    {
      std::lock_guard<std::mutex> m(chunk_promises_mutex);

      if (read_ahead_promises > resolved_promises) {
        chunk_promises.front().set_value(FWD(chunk));
        chunk_promises.pop_front();
        --read_ahead_promises;
        decrementProcessedChunks();
      } else {
        auto promise = chunk_promise();
        promise.set_value(FWD(chunk));
        chunk_promises.push_back(std::move(promise));
        ++resolved_promises;
      }
    }
  }

  void onReject() { decrementProcessedChunks(); }

  void incrementProcessedChunks() {
    std::lock_guard<std::mutex> m(drained_mutex);
    ++processed_chunks;
    checkDrain();
  };

  void decrementProcessedChunks() {
    std::lock_guard<std::mutex> m(drained_mutex);
    --processed_chunks;
    checkDrain();
  };

  void checkDrain() {
    if (LimitExceeded()) {
      DrainNeeded();
    } else {
      SetDrained();
    }
  }

  bool LimitExceeded() { return processed_chunks >= max_parallel; }

  void DrainNeeded() {
    if (drained_promise_) return;
    drained_promise_ = std::make_unique<std::promise<void>>();
    drained_sfuture_ = drained_promise_->get_future().share();
    drained = false;
  }
  void SetDrained() {
    if (!drained_promise_) return;
    drained_promise_->set_value();
    drained_promise_.reset();
    drained = true;
  }

 private:
  const unsigned int max_parallel;
  std::tuple<TransformChain...> transforms_;

  std::future<void> last_processing_future_;
  std::atomic_bool ended_;

  std::mutex chunk_promises_mutex;
  std::list<chunk_promise> chunk_promises;
  size_t resolved_promises;
  size_t read_ahead_promises;

  std::mutex drained_mutex;
  std::unique_ptr<std::promise<void>> drained_promise_;
  std::shared_future<void> drained_sfuture_;
  std::size_t processed_chunks;
  bool drained;
};

template <typename Input = void, typename Output = Input>
inline auto Ifca(unsigned int max_parallel = maxParallel()) {
  return IfcaImpl<Input, Output>(max_parallel);
}

}  // namespace ifca

#endif  // IFCA_H
