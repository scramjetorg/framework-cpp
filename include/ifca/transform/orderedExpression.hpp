#ifndef ORDERED_EXPRESSION_H
#define ORDERED_EXPRESSION_H

#include <type_traits>

#include "helpers/crtp.hpp"
#include "transformExpression.hpp"

namespace ifca {

template <typename Impl = CrtpFinalImpl>
class OrderedExpression
    : public CrtpImpl<OrderedExpression, Impl, TransformExpression> {
 public:
  using BaseType = CrtpImpl<OrderedExpression, Impl, TransformExpression>;
  using ExactType = typename BaseType::ExactType;
};

template <typename Transform>
using IsOrderedExpression =
    std::enable_if_t<is_crtp_interface_of<OrderedExpression, Transform>::value,
                     bool>;

}  // namespace ifca

#endif  // ORDERED_EXPRESSION_H