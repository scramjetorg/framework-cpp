#ifndef TRANSFORM_CHAIN_H
#define TRANSFORM_CHAIN_H

#include "transformExpression.hpp"

namespace ifca {

template <typename HeadTransform, typename TailTransform>
class TransformChain
    : public TransformExpression<TransformChain<HeadTransform, TailTransform>> {
 public:
  HeadTransform t1_;
  TailTransform t2_;

  TransformChain(HeadTransform&& transform1, TailTransform&& transform2_)
      : t1_(FWD(transform1)), t2_(FWD(transform2_)) {}

  template <typename... Values>
  void operator()(Values&&... values) {
    t1_.template operator()<Values...>(FWD(values)..., t2_);
  }
};

template <typename HeadTransform, typename TailTransform,
          IsTransformExpression<std::remove_reference_t<HeadTransform>> = true,
          IsTransformExpression<std::remove_reference_t<TailTransform>> = true>
auto operator>>=(HeadTransform&& e1, TailTransform&& e2) {
  return TransformChain<HeadTransform, TailTransform>(FWD(e1), FWD(e2));
}

}  // namespace ifca

#endif  // TRANSFORM_CHAIN_H