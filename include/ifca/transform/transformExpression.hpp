#ifndef TRANSFORM_EXPRESSION_H
#define TRANSFORM_EXPRESSION_H

#include <type_traits>

#include "helpers/crtp.hpp"

namespace ifca {

template <typename T>
class TransformExpression : public crtp<T> {
 public:
  using BaseType = crtp<T>;
  using ExactType = typename BaseType::ExactType;

  template <typename... Args>
  void operator()(Args... args) {
    this->derived()(args...);
  }
};

template <typename Transform>
using IsTransformExpression = std::enable_if_t<
    is_crtp_interface_of<TransformExpression, Transform>::value, bool>;

}  // namespace ifca

#endif  // TRANSFORM_EXPRESSION_H