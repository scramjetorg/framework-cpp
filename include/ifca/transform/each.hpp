#ifndef EACH_H
#define EACH_H

#include "helpers/FWD.hpp"
#include "transformExpression.hpp"

namespace ifca {

template <typename Function>
class EachTransform : public TransformExpression<EachTransform<Function>> {
 public:
  explicit EachTransform(Function& function) : func_(function) {}

  template <typename... Values, typename TailTransform >
  void operator()(Values&&... values, TailTransform  transform) {
    transform(FWD(func_(values...)));
  }

 private:
  Function& func_;
};

template <typename Function>
auto Each(Function& function) {
  return EachTransform<Function>(function);
}

}  // namespace ifca

#endif  // EACH_H