#ifndef CRTP_H
#define CRTP_H

#include <type_traits>

// The Curiously Recurring Template Pattern

/**
 * @brief Class for multilevel inheritance.
 *
 */
struct CrtpFinalImpl;

/**
 * @brief Helper name for multileveled inheritance hierarchy
 *
 * @tparam This Name of current class
 * @tparam Impl Type of current class parameters
 * @tparam Super Type of Base class
 */
template <template <typename> class This, typename Impl,
          template <typename> class Super>
using CrtpImpl = Super<std::conditional_t<std::is_same_v<Impl, CrtpFinalImpl>,
                                          This<CrtpFinalImpl>, Impl>>;

template <typename T>
struct crtp {
  // used to get type needed by friend declarations and SFINAE
  using exact_type = T;
  exact_type& derived() { return static_cast<exact_type&>(*this); }
  exact_type const& derived() const {
    return static_cast<exact_type const&>(*this);
  }
};

/**
 * @brief Compile-time check if class implements given crtp interface
 *
 * @tparam CrtpInterface checked class as base
 * @tparam Derived checked class if implements base
 */
template <template <typename> class CrtpInterface, typename Derived,
          typename Enable = void>
struct is_crtp_interface_of : std::false_type {};

template <template <typename> class CrtpInterface, typename Derived>
struct is_crtp_interface_of<
    CrtpInterface, Derived,
    std::enable_if_t<std::is_same_v<CrtpInterface<CrtpFinalImpl>, Derived> ||
                     std::is_base_of_v<
                         CrtpInterface<typename Derived::exact_type>, Derived>>>
    : std::true_type {};

#endif  // CRTP_H
