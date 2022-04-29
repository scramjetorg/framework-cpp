#ifndef IFCA_H
#define IFCA_H

#include <tuple>
#include <type_traits>

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/asyncForwarder.hpp"
#include "helpers/threadList.hpp"
#include "ifca/drain.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/maxParallel.hpp"
#include "ifca/transform/isTransformExpression.hpp"
#include "ifca/types.hpp"

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
      : drain_state_(max_parallel), ended_(false) {
    LOG_INFO() << "Ifca default constructor";
    std::promise<void> first_processing_promise;
    first_processing_promise.set_value();
    last_processing_future_ = first_processing_promise.get_future();
  };

  template <typename Input, typename Output, typename... Transforms,
            typename = std::enable_if_t<std::is_same_v<
                std::tuple<Transforms...>, std::tuple<TransformChain...>>>>
  explicit IfcaImpl(IfcaImpl<Input, Output, Transforms...>&& ifca)
      : transforms_(std::move(ifca.transforms_)),
        drain_state_(std::move(ifca.drain_state_)),
        last_processing_future_(std::move(ifca.last_processing_future_)),
        ended_(std::move(ifca.ended_)) {
    LOG_INFO() << "Ifca move constructor";
  }

  template <typename Input, typename Output, typename... Transforms,
            typename Transform>
  explicit IfcaImpl(IfcaImpl<Input, Output, Transforms...>&& ifca,
                    Transform&& transform)
      : transforms_(detail::ForwardTransformChain(std::move(ifca.transforms_),
                                                  FWD(transform))),
        drain_state_(std::move(ifca.drain_state_)),
        last_processing_future_(std::move(ifca.last_processing_future_)),
        ended_(std::move(ifca.ended_)) {
    LOG_INFO() << "Ifca move with transform constructor";
  }

  IfcaImpl(const IfcaImpl&) = delete;
  IfcaImpl(IfcaImpl&&) = default;
  IfcaImpl& operator=(const IfcaImpl&) = delete;
  IfcaImpl& operator=(IfcaImpl&&) = delete;

  ~IfcaImpl() {
    if (!ended_) end();
    if (!last_processing_future_.valid()) return;
    last_processing_future_.wait();
  };

  template <typename Input = input_type>
  std::enable_if_t<!std::is_void_v<Input>, drain_sfuture> write(Input&& chunk) {
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
        [this](Input&& chunk, std::future<void>&& previous_processing_future) {
          if constexpr (sizeof...(TransformChain) > 0) {
            runTransforms(
                FWD(chunk), previous_processing_future,
                std::make_index_sequence<sizeof...(TransformChain)>{});
          } else {
            onResolve(FWD(chunk), previous_processing_future);
          }
        },
        async_forwarder<Input>(chunk), std::move(last_processing_future_));

    return drain_state_;
  }

  template <typename Input = input_type>
  std::enable_if_t<!std::is_void_v<Input>, chunk_future> read() {
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
    for (auto&& read_ahead : read_ahead_promises_) {
      setReadEnd(read_ahead);
    }
  }

  template <typename, typename, typename...>
  friend class IfcaImpl;

 protected:
  void setReadEnd(chunk_promise& promise) {
    try {
      throw ReadEnd();
    } catch (const ReadEnd&) {
      promise.set_exception(std::current_exception());
    }
  }

  template <typename Chunk, std::size_t... Is>
  void runTransforms(Chunk&& chunk,
                     std::future<void>& previous_processing_future,
                     std::index_sequence<Is...>) {
    runTransformsImpl(FWD(chunk), previous_processing_future,
                      std::get<Is>(transforms_)...);
  }

  template <typename Chunk, typename FirstTransform,
            typename... RestOfTransforms>
  void runTransformsImpl(Chunk&& chunk,
                         std::future<void>& previous_processing_future,
                         FirstTransform&& firstTransform,
                         RestOfTransforms&&... restOfTransforms) {
    auto&& onResolveCallback = [&previous_processing_future,
                                this](auto&& resolvedValue) {
      onResolve(FWD(resolvedValue), previous_processing_future);
    };
    auto&& onRejectCallback = [this] { onReject(); };
    firstTransform(FWD(chunk), FWD(onResolveCallback), FWD(onRejectCallback),
                   FWD(restOfTransforms)...);
  }

  template <typename Chunk>
  void onResolve(Chunk&& chunk, std::future<void>& previous_processing_future) {
    previous_processing_future.wait();
    auto&& result_promise = processing_promises_.take_front();
    result_promise.set_value(FWD(chunk));
    drain_state_.ChunkFinishedProcessing();
  }

  void onReject() {
    // TODO: remove one of processing_promises_?
    drain_state_.ChunkFinishedProcessing();
  }

 private:
  std::tuple<TransformChain...> transforms_;

  DrainState drain_state_;
  std::future<void> last_processing_future_;
  bool ended_;
  ThreadList<chunk_promise> processing_promises_;
  std::list<chunk_promise> read_ahead_promises_;
  std::list<chunk_future> read_futures_;
};

template <typename Input = void, typename Output = Input>
inline auto Ifca(unsigned int max_parallel = maxParallel()) {
  return IfcaImpl<Input, Output>(max_parallel);
}

}  // namespace ifca

#endif  // IFCA_H
