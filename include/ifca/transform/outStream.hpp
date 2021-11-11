#ifndef OUTSTREAM_H
#define OUTSTREAM_H

#include <functional>

#include "helpers/FWD.hpp"
#include "transformExpression.hpp"

namespace ifca {

// TODO: change according to spec
template <typename OutStream>
class OutStreamTransform
    : public CrtpImpl<OutStreamTransform, OutStream, TransformExpression> {
 public:
  explicit OutStreamTransform(OutStream& outStream) : out_stream_(outStream) {}

  template <typename T>
  void operator()(T&& value) {
    out_stream_.get() << FWD(value);
  }

 private:
  std::reference_wrapper<OutStream> out_stream_;
};

template <typename OutputStream>
auto OutStream(OutputStream& outstream) {
  return OutStreamTransform<OutputStream>(outstream);
}

}  // namespace ifca

#endif  // OUTSTREAM_H