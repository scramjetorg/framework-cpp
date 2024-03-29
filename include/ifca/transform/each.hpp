#ifndef EACH_H
#define EACH_H

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
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
  using output_type = input_type;

  explicit EachTransform(Function& function) : func_(function) {}
  virtual ~EachTransform() = default;

  template <typename Chunk, typename ResolveCallback, typename RejectCallback,
            typename NextTransform, typename... TransformChain>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve,
                  RejectCallback&& reject, NextTransform&& next,
                  TransformChain&&... transforms) {
    func_(chunk);
    next(FWD(chunk), FWD(resolve), FWD(reject), FWD(transforms)...);
  }

  template <typename Chunk, typename ResolveCallback, typename RejectCallback>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve, RejectCallback&&) {
    func_(chunk);
    resolve(FWD(chunk));
  }

 private:
  Function func_;
};

template <typename Function>
auto each(Function&& function) {
  return EachTransform<Function>(function);
}

}  // namespace ifca

#endif  // EACH_H