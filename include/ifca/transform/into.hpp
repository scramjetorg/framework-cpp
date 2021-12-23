#ifndef INTO_H
#define INTO_H

#include <functional>

#include "helpers/FWD.hpp"
#include "helpers/crtp.hpp"
#include "orderedExpression.hpp"

namespace ifca {

template <typename Function>
class IntoTransform
    : public CrtpImpl<IntoTransform, Function, OrderedExpression> {
 public:
  using BaseType = CrtpImpl<IntoTransform, Function, OrderedExpression>;
  using ExactType = typename BaseType::ExactType;
  explicit IntoTransform(Function& function) : function_(function) {}

  template <typename... Values, typename TailTransform>
  void operator()(Values&&... values, TailTransform tailTransform) {
    function_(values...);  // FIXME: fix forwarding as tuple to save us from
                           // unwanted copies
    tailTransform(FWD(values)...);
  }

 private:
  Function function_;  // TODO: add proper forwarding
};

template <typename OutStream>
auto Into(OutStream& outstream) {
  return IntoTransform<OutStream>(outstream);
}

}  // namespace ifca

#endif  // INTO_H