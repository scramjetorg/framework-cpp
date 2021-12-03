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
  using base_type = CrtpImpl<OrderedExpression, Impl, TransformExpression>;
  using exact_type = typename base_type::exact_type;
};

template <typename Transform>
using IsOrderedExpression =
    std::enable_if_t<is_crtp_interface_of<OrderedExpression, Transform>::value,
                     bool>;

}  // namespace ifca

#endif  // ORDERED_EXPRESSION_H