#ifndef FILTER_H
#define FILTER_H

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/functionTraits.hpp"
#include "transformExpression.hpp"

namespace ifca {

// TODO: SFINAE for Predicate
template <typename Predicate>
class FilterTransform
    : public CrtpImpl<FilterTransform, Predicate, TransformExpression> {
 public:
  using base_type = CrtpImpl<FilterTransform, Predicate, TransformExpression>;
  using exact_type = typename base_type::exact_type;
  using input_type = typename function_traits<Predicate>::template arg<0>;
  using output_type = typename function_traits<Predicate>::template arg<0>;
  explicit FilterTransform(Predicate& predicate) : predicate_(predicate){};

  template <typename Value>
  Value operator()(Value&& value) {
    if (predicate_(value)) {
      return FWD(value);
    }
    // FIXME: Get rid of throw for optimalization purpose
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