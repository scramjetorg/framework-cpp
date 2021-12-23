#ifndef FUNCTION_TRAITS_H
#define FUNCTION_TRAITS_H

#include <functional>

template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
  static const size_t nargs = sizeof...(Args);

  using return_type = R;

  template <size_t i>
  using arg = typename std::tuple_element<i, std::tuple<Args...>>::type;
};

#endif  // FUNCTION_TRAITS_H