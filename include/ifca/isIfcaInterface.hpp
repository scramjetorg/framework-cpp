#ifndef IS_IFCA_INTERFACE_H
#define IS_IFCA_INTERFACE_H

#include <type_traits>

#include "ifca/ifca.hpp"

namespace ifca {

/**
 * @brief Compile-time check for Ifca classes
 *
 * @tparam IfcaInterface Checked class
 * @tparam Enable
 */
template <typename IfcaInterface, typename Enable = void>
struct is_ifca_interface : std::false_type {};

template <template <typename, typename, typename...> typename IfcaInterface,
          typename In, typename Out, typename... TransformChain>
struct is_ifca_interface<
    IfcaInterface<In, Out, TransformChain...>,
    std::enable_if_t<std::is_same_v<IfcaInterface<In, Out, TransformChain...>,
                                    IfcaImpl<In, Out, TransformChain...>>>>
    : std::true_type {};

template <typename IfcaInterface>
inline constexpr bool is_ifca_interface_v =
    is_ifca_interface<IfcaInterface>::value;

}  // namespace ifca

#endif  // IS_IFCA_INTERFACE_H