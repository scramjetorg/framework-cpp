#ifndef REDUCE_H
#define REDUCE_H

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/functionTraits.hpp"
#include "transformExpression.hpp"

namespace ifca {

// TODO: SFINAE for Function
template <typename Function>
class ReduceTransform
    : public CrtpImpl<ReduceTransform, Function, TransformExpression> {
 public:
  using base_type = CrtpImpl<ReduceTransform, Function, TransformExpression>;
  using exact_type = typename base_type::exact_type;
  using input_type = typename function_traits<Function>::template arg<0>;
  using output_type = typename function_traits<Function>::return_type;

  explicit ReduceTransform(Function& function) : func_(function) {}

  template <typename Chunk, typename ResolveCallback, typename RejectCallback,
            typename NextTransform, typename... TransformChain>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve,
                  RejectCallback&& reject, NextTransform&& next,
                  TransformChain&&... transforms) {
    next(std::forward<Function>(func_)(FWD(chunk)), FWD(resolve), FWD(reject),
         FWD(transforms)...);
  }

  template <typename Chunk, typename ResolveCallback, typename RejectCallback>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve, RejectCallback&&) {
    resolve(std::forward<Function>(func_)(FWD(chunk)));
  }

 private:
  Function func_;
};

template <typename Function>
auto reduce(Function&& function) {
  return ReduceTransform<Function>(function);
}

}  // namespace ifca

#endif  // REDUCE_H