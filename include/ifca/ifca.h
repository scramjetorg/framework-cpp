#pragma once

#include <type_traits>
#include <future>
#include <optional>

namespace ifca {

  enum FuncType {
      Type1T,
      Type2T,
  };

  namespace transform {
    template<typename T, typename S>
    using synchronous = S (*)(T);

    template<typename T, typename S>
    using async_promise = std::promise<S> (*)(T);

    template<typename T, typename S>
    using async_future = std::future<S> (*)(T);
  }

  template <typename T, typename S, typename I = void>
  class IFCA {
    static_assert(std::is_base_of<IFCA, I>::value, "I must inherit from IFCA");

  public:
    IFCA(int maxParallel, transform::synchronous<T,S> *op);
    IFCA(int maxParallel, transform::async_promise<T,S> *op);
    IFCA(int maxParallel, transform::async_future<T,S> *op);

    std::optional<std::promise<void>> write(T chunk);
    std::optional<std::promise<void>> end();

    std::promise<std::optional<S>> read();

    template<typename W>
    IFCA<T,W,IFCA<T,S,I>> addTransform(transform::synchronous<S,W> *op);
    template<typename W>
    IFCA<T,W,IFCA<T,S,I>> addTransform(transform::async_promise<S,W> *op);
    template<typename W>
    IFCA<T,W,IFCA<T,S,I>> addTransform(transform::async_future<S,W> *op);

    I removeTransform();
  };

}  // namespace greeter

