#ifndef EACH_H
#define EACH_H

#include "helpers/FWD.hpp"
#include "helpers/functionTraits.hpp"
#include "transformExpression.hpp"

namespace ifca {

// TODO: SFINAE for Function
template <typename Function>
class EachTransform
    : public CrtpImpl<EachTransform, Function, TransformExpression> {
 public:
  using base_type = CrtpImpl<EachTransform, Function, TransformExpression>;
  using exact_type = typename base_type::exact_type;

  using input_type = typename function_traits<Function>::template arg<0>;
  using result_type = typename function_traits<Function>::result_type;

  explicit EachTransform(Function& function) : func_(function) {}

  template <typename Value>
  result_type operator()(Value&& value) {
    return FWD(func_(value));
  }

 private:
  Function func_;
};

template <typename Function>
auto Each(Function& function) {
  return EachTransform<Function>(function);
}

}  // namespace ifca

#endif  // EACH_H