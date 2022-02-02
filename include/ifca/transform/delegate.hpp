#ifndef DELEGATE_H
#define DELEGATE_H

#include <functional>

#include "helpers/Logger/logger.hpp"
// #include "helpers/FWD.hpp"
#include "helpers/functionTraits.hpp"

namespace detail {

/*! @brief Used to wrap a function or a member of a specified type. */
template <auto>
struct connect_arg_t {};

/*! @brief Constant of type connect_arg_t used to disambiguate calls. */
template <auto Func>
inline constexpr connect_arg_t<Func> connect_arg{};

// template <typename Ret, typename... Args>
// auto to_function_pointer(Ret (*)(Args...)) -> Ret (*)(Args...);

template <typename>
struct delegate;

template <typename R, typename... Args>
class delegate<R(Args...)> {
 public:
  using storage_type = std::aligned_storage_t<sizeof(void *), alignof(void *)>;
  using proto_fn_type = R(storage_type &, Args...);

  delegate() : fn{nullptr} {}

  // template <auto Function>
  // delegate(connect_arg_t<Function>) : delegate{} {
  //   connect<Function>();
  // }

  // Member functions and curried functions
  template <auto Candidate, typename Type>
  void connect(Type value_or_instance) {
    static_assert(sizeof(Type) <= sizeof(void *));
    static_assert(std::is_trivially_copyable_v<Type> and
                  std::is_trivially_destructible_v<Type>);
    static_assert(
        std::is_invocable_r_v<R, decltype(Candidate), Type &, Args...>);
    LOG_DEBUG() << "Member func";

    new (&storage) Type{value_or_instance};

    fn = [](storage_type &storage, Args... args) -> R {
      Type value_or_instance = *reinterpret_cast<Type *>(&storage);
      return std::invoke(Candidate, value_or_instance, args...);
    };
  }

  // Free functions
  template <auto Function>
  void connect() {
    static_assert(std::is_invocable_r_v<R, decltype(Function), Args...>);
    LOG_DEBUG() << "Free func";
    new (&storage) void *{nullptr};
    fn = [](storage_type &, Args... args) -> R {
      return std::invoke(Function, args...);
    };
  }

  // Lambdas and functors
  template <typename Invokable>
  void connect(Invokable invokable) {
    static_assert(sizeof(Invokable) < sizeof(void *));
    static_assert(std::is_class_v<Invokable>);
    static_assert(std::is_trivially_destructible_v<Invokable>);
    static_assert(std::is_invocable_r_v<R, Invokable, Args...>);

    LOG_DEBUG() << "Lambda func";
    new (&storage) Invokable{std::move(invokable)};

    fn = [](storage_type &storage, Args... args) -> R {
      Invokable &invokable = *reinterpret_cast<Invokable *>(&storage);
      return std::invoke(invokable, args...);
    };
  }

  R operator()(Args... args) { return fn(storage, args...); }

 private:
  mutable storage_type storage;
  proto_fn_type *fn;
};

// template <auto Function>
// delegate(connect_arg_t<Function>)
//     -> delegate<std::remove_pointer_t<decltype(to_function_pointer(Function))>>;

}  // namespace detail

#endif  // DELEGATE_H