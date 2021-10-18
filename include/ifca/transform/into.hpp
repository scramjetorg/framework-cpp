#ifndef INTO_H
#define INTO_H

#include <functional>

#include "helpers/FWD.hpp"
#include "transformExpression.hpp"

namespace ifca {

// TODO: change according to spec
template <typename OutStream>
class IntoTransform : public TransformExpression<IntoTransform<OutStream>> {
 public:
  explicit IntoTransform(OutStream& outStream) : out_stream_(outStream) {}

  template <typename T>
  void operator()(T&& value) {
    out_stream_.get() << FWD(value);
  }
  template <typename... Values, typename TailTransform >
  void operator()(Values&&... values, TailTransform  tailTransform) {
    // out_stream_.get() << (values...);
    // tailTransform(FWD(values...));
  }

 private:
  std::reference_wrapper<OutStream> out_stream_;
};

template <typename OutStream>
auto Into(OutStream& outstream) {
  return IntoTransform<OutStream>(outstream);
}

}  // namespace ifca

#endif  // INTO_H