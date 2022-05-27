#ifndef IS_TRANSFORM_EXPRESSION_H
#define IS_TRANSFORM_EXPRESSION_H

#include <type_traits>

#include "helpers/crtp.hpp"
#include "ifca/transform/transformExpression.hpp"

namespace ifca {

/**
 * @brief Compile-time check for derived from TransformExpression classes
 *
 * @tparam Transform to check
 */
template <typename Transform>
using is_transform_expression =
    is_crtp_interface_of<TransformExpression, std::decay_t<Transform>>;

template <typename Transform>
inline constexpr bool is_transform_expression_v =
    is_transform_expression<Transform>::value;

}  // namespace ifca

#endif  // IS_TRANSFORM_EXPRESSION_H