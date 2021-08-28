#pragma once

#include <future>
#include <optional>

namespace ifca {

  template<typename T, typename S>
  using operation = S (*)(T);

  template <typename T, typename S>
  class IFCA {
    int m_maxParallel;
    std::array<operation<T,S>, 0>* f_op;

  public:
    IFCA(int maxParallel, operation<T,S> *op);

    std::optional<std::promise<void>> write(T chunk);
    std::optional<std::promise<void>> end();

    std::promise<std::optional<S>> read();

  };

}  // namespace greeter

