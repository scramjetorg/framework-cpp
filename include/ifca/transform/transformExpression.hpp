#ifndef TRANSFORM_EXPRESSION_H
#define TRANSFORM_EXPRESSION_H

#include <type_traits>

#include "helpers/crtp.hpp"

namespace ifca {

template <typename T>
class TransformExpression : public crtp<T> {
 public:
  using base_type = crtp<T>;
  using exact_type = typename base_type::exact_type;

  template <typename... Args>
  void operator()(Args... args) {
    this->derived()(args...);
  }
};

template <typename Transform>
using IsTransformExpression =
    is_crtp_interface_of<TransformExpression, Transform>;

template <typename Transform>
using EnableIfTransformExpression =
    std::enable_if_t<IsTransformExpression<Transform>::value, Transform>;

}  // namespace ifca

#endif  // TRANSFORM_EXPRESSION_H