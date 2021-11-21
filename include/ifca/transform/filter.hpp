#ifndef FILTER_H
#define FILTER_H

#include "helpers/FWD.hpp"
#include "helpers/functionTraits.hpp"
#include "transformExpression.hpp"

namespace ifca {

// TODO: SFINAE for Predicate
template <typename Predicate>
class FilterTransform
    : public CrtpImpl<FilterTransform, Predicate, TransformExpression> {
 public:
  using BaseType = CrtpImpl<FilterTransform, Predicate, TransformExpression>;
  using ExactType = typename BaseType::ExactType;
  using input_type = typename function_traits<Predicate>::template arg<0>;
  using result_type = input_type;
  explicit FilterTransform(Predicate& predicate) : predicate_(predicate){};

  template <typename Value>
  result_type operator()(Value&& value) {
    if (predicate_(value)) {
      return value;
    }
    throw std::invalid_argument("Filtered");
  }

 private:
  Predicate predicate_;
};

template <typename Predicate>
auto Filter(Predicate& predicate) {
  return FilterTransform<Predicate>(predicate);
}

}  // namespace ifca

#endif  // FILTER_H