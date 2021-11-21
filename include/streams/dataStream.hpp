#ifndef DATA_STREAM_H
#define DATA_STREAM_H

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"

namespace stream {

template <typename In = void, typename Out = void>
class DataStreamImpl {
 public:
  // TODO: check if init list causes construct and move of empty IFCA
  DataStreamImpl(ifca::Ifca<In, Out>&& ifca = ifca::Ifca<In, Out>())
      : ifca_(FWD(ifca)) {
    LOG_DEBUG() << "DataStreamImpl created";
  }
  ~DataStreamImpl() { LOG_DEBUG() << "DataStreamImpl removed"; }

  template <typename Predicate>
  auto filter(Predicate&& predicate) {
    auto&& newIfca = ifca_ + ifca::Filter(FWD(predicate));
    using ifca_in =
        typename std::remove_reference_t<decltype(newIfca)>::in_type;
    using ifca_out =
        typename std::remove_reference_t<decltype(newIfca)>::out_type;
    return DataStreamImpl<ifca_in, ifca_out>(FWD(newIfca));
  }

  template <typename Function>
  auto each(Function&& function) {
    auto&& newIfca = ifca_ + ifca::Each(FWD(function));
    using ifca_in =
        typename std::remove_reference_t<decltype(newIfca)>::in_type;
    using ifca_out =
        typename std::remove_reference_t<decltype(newIfca)>::out_type;
    return DataStreamImpl<ifca_in, ifca_out>(FWD(newIfca));
  }

  template <typename Value>
  auto operator()(Value&& value) {
    return ifca_(FWD(value));
  }

 private:
  ifca::Ifca<In, Out> ifca_;
};

inline auto DataStream() { return DataStreamImpl<>(); }

}  // namespace stream

#endif  // DATA_STREAM_H
