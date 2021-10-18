#ifndef FILTER_H
#define FILTER_H

#include "helpers/FWD.hpp"
#include "transformExpression.hpp"

namespace ifca {

template <typename Predicate>
class FilterTransform : public TransformExpression<FilterTransform<Predicate>> {
 public:
  explicit FilterTransform(Predicate& predicate) : predicate_(predicate){};

  template <typename... Values, typename TailTransform>
  void operator()(Values&&... values, TailTransform tail_transform) {
    if (predicate_(values...)) {
      tail_transform(FWD(values)...);
    }
  }

 private:
  Predicate& predicate_;
};

template <typename Predicate>
auto Filter(Predicate& predicate) {
  return FilterTransform<Predicate>(predicate);
}

}  // namespace ifca

#endif  // FILTER_H