#ifndef FUNCTION_TRAITS_H
#define FUNCTION_TRAITS_H

#include <functional>

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())> {};

template <>
struct function_traits<void> {};

template <typename R, typename... Args>
struct function_traits<R(Args...)> {
  static const size_t nargs = sizeof...(Args);

  using return_type = R;

  template <size_t i>
  using arg = typename std::tuple_element<i, std::tuple<Args...>>::type;
};

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)> {};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : public function_traits<R(Args...)> {
  using class_type = C&;
};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const>
    : public function_traits<R(Args...)> {
  using class_type = const C&;
};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) volatile>
    : public function_traits<R(Args...)> {
  using class_type = volatile C&;
};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile>
    : public function_traits<R(Args...)> {
  using class_type = const volatile C&;
};

template <typename FunctionType>
struct function_traits<std::function<FunctionType>>
    : public function_traits<FunctionType> {};

template <typename T>
struct function_traits<T&> : public function_traits<T> {};
template <typename T>
struct function_traits<const T&> : public function_traits<T> {};
template <typename T>
struct function_traits<volatile T&> : public function_traits<T> {};
template <typename T>
struct function_traits<const volatile T&> : public function_traits<T> {};
template <typename T>
struct function_traits<T&&> : public function_traits<T> {};
template <typename T>
struct function_traits<const T&&> : public function_traits<T> {};
template <typename T>
struct function_traits<volatile T&&> : public function_traits<T> {};
template <typename T>
struct function_traits<const volatile T&&> : public function_traits<T> {};

#endif  // FUNCTION_TRAITS_H