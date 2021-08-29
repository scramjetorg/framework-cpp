#pragma once

#include <type_traits>
#include <future>
#include <optional>
#include <ifca/operation.h>

namespace ifca {

  template <typename T, typename S, typename I = void>
  class IFCA {
    static_assert(std::is_base_of<IFCA, I>::value || std::is_void<I>::value, "I must inherit from IFCA or be void");

  private:
    int n_maxParallel;

  public:
    IFCA(int maxParallel, typename operation<T, S>::synchronous op);
    IFCA(int maxParallel, typename operation<T, S>::async_promise op);
    IFCA(int maxParallel, typename operation<T, S>::async_future op);
    IFCA(int maxParallel, operation<T,S> op);

    std::optional<std::promise<void>> write(T chunk);
    std::optional<std::promise<void>> end();

    std::promise<std::optional<S>> read();

    template <typename W>
    IFCA<T,W,IFCA<T,S,I>> addTransform(typename operation<S, W>::synchronous op);
    template <typename W>
    IFCA<T,W,IFCA<T,S,I>> addTransform(typename operation<S, W>::async_promise op);
    template <typename W>
    IFCA<T,W,IFCA<T,S,I>> addTransform(typename operation<S, W>::async_future op);

    I removeTransform();
  };

}  // namespace greeter

