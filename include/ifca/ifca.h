#pragma once

#include <type_traits>
#include <future>
#include <optional>

namespace ifca {

  enum FuncType {
      Type1T,
      Type2T,
  };

  template <typename T, typename S, typename W = void, typename I = void>
  class IFCA {
    static_assert(std::is_base_of<IFCA, I>::value || std::is_void<I>::value, "I must inherit from IFCA or be void");

    using synchronous = std::function<S(T)>;
    using async_future = std::function<std::future<S>(T)>;
    using async_promise = std::function<std::promise<S>(T)>;

  public:
    IFCA(int maxParallel, synchronous op);
    IFCA(int maxParallel, std::function<async_promise> op);
    IFCA(int maxParallel, std::function<async_future> op);

    std::optional<std::promise<void>> write(T chunk);
    std::optional<std::promise<void>> end();

    std::promise<std::optional<S>> read();

    IFCA<T,W,IFCA<T,S,I>> addTransform(std::function<auto (S) -> W> op);
    IFCA<T,W,IFCA<T,S,I>> addTransform(std::function<auto (std::promise<S>) -> T> op);
    IFCA<T,W,IFCA<T,S,I>> addTransform(std::function<auto (std::future<S>) -> T> op);

    I removeTransform();
  };

}  // namespace greeter

