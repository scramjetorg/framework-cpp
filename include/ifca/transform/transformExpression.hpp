#ifndef TRANSFORM_EXPRESSION_H
#define TRANSFORM_EXPRESSION_H

#include <type_traits>

namespace ifca {

template <typename T>
class TransformExpression : public crtp<T, TransformExpression> {
 public:
  template <typename... Args>
  void operator()(Args... args) {
    this->derived()(args...);
  }

 private:
  TransformExpression(){};
  friend T;
};

template <typename Transform>
using IsTransformExpression = std::enable_if_t<
    std::is_base_of<TransformExpression<Transform>, Transform>::value, bool>;

}  // namespace ifca

#endif  // TRANSFORM_EXPRESSION_H