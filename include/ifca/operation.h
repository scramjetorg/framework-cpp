#pragma once

#include <type_traits>
#include <future>
#include <optional>

namespace ifca {

  template <typename Tx, typename Sx>
  class operation {

    public:
    using synchronous = std::function<Sx(Tx)>;
    using async_future = std::function<std::future<Sx>(Tx)>;
    using async_promise = std::function<std::promise<Sx>(Tx)>;

    std::optional<synchronous> s;
    std::optional<async_future> f;
    std::optional<async_promise> p;
  };

}