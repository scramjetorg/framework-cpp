#ifndef IS_IFCA_INTERFACE_H
#define IS_IFCA_INTERFACE_H

#include <type_traits>

namespace ifca {

/**
 * @brief Compile-time check for derived from Ifca classes
 *
 * @tparam Derived Checked class
 * @tparam Enable
 */
template <typename Derived, typename Enable = void>
struct is_ifca_interface : std::false_type {};

// template <typename Derived>
// struct is_ifca_interface<
//     Derived, std::enable_if_t<std::is_base_of_v<
//                  IfcaMethods<typename Derived::input_type,
//                              typename Derived::output_type, Derived>,
//                  Derived>>> : std::true_type {};

template <typename Derived>
inline constexpr bool is_ifca_interface_v = is_ifca_interface<Derived>::value;

}  // namespace ifca

#endif  // IS_IFCA_INTERFACE_H