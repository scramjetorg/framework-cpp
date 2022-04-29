#ifndef DATA_STREAM_H
#define DATA_STREAM_H

#include <type_traits>

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/isStlContainer.hpp"
#include "ifca/ifca.hpp"
#include "ifca/isIfcaInterface.hpp"
#include "ifca/maxParallel.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/transform/isTransformExpression.hpp"
#include "ifca/transform/map.hpp"
#include "ifca/transform/reduce.hpp"
#include "ifca/types.hpp"

namespace stream {

template <typename In = void, typename Out = In, typename... Transforms>
class DataStream {
 public:
  explicit DataStream(unsigned int max_parallel = maxParallel())
      : ifca(max_parallel)
  // corked(false)
  {
    LOG_DEBUG() << "DataStream deafult constructor";
  };
  ~DataStream() = default;

  template <typename Source,
            typename = std::enable_if_t<is_stl_container_v<Source>>>
  decltype(auto) from(Source&& source) {
    using Input = typename std::decay_t<Source>::value_type;
    if constexpr (!std::is_same_v<Input, In>) {
      LOG_DEBUG() << "Other In";
      return DataStream<Input, Out, Transforms...>(std::move(*this))
          .from<Source>(FWD(source));
    } else {
      fromContainer<Input>(FWD(source));
      return *this;
    }
  }

  template <typename Input, typename Source,
            typename = std::enable_if_t<
                std::is_base_of_v<std::istream, std::decay_t<Source>>>>
  decltype(auto) from(Source&& source) {
    if constexpr (!std::is_same_v<Input, In>) {
      LOG_DEBUG() << "Other In";
      return DataStream<Input, Out, Transforms...>(std::move(*this))
          .from<Input>(FWD(source));
    } else {
      fromStream<Input>(FWD(source));
      return *this;
    }
  }

  // template <typename Input = In>
  // std::enable_if_t<!std::is_void_v<Input>, std::shared_future<void>> write(
  //     Input&& chunk) {
  //   uncork();
  //   return ifca.write(FWD(chunk));
  // };

  //// std::future<Out>
  // std::future<Out> read() { return ifca.read(); };

  // flow controll
  void pause() { cork(); };
  void resume() { uncork(); };
  void end() { ifca.end(); };

  // transforms
  // TODO: check if functional object?
  template <typename Function>
  auto each(Function&& function) {
    return addTransform(ifca::each(FWD(function)));
  };
  template <typename Function>
  auto map(Function&& function) {
    return addTransform(ifca::map(FWD(function)));
  };
  template <typename Function>
  auto filter(Function&& function) {
    return addTransform(ifca::filter(FWD(function)));
  };
  auto pipe(){};
  template <typename Function>
  auto reduce(Function&& function) {
    return addTransform(ifca::reduce(FWD(function)));
  };

  // Output methods
  template <typename T, size_t size>
  void toArray(T (&array)[size]) {
    auto it = 0;
    while (it < size) {
      array[it++] = it;
    }
  };

  template <typename Container,
            typename = std::enable_if_t<is_stl_container_v<Container>>>
  void toContainer(Container&& container) {
    using value_type = typename std::decay_t<Container>::value_type;
    value_type value = 1;
    container.insert(container.cend(), value);
  };

  template <typename Stream, typename = std::enable_if_t<std::is_base_of_v<
                                 std::ostream, std::decay_t<Stream>>>>
  void toOutStream(Stream&& stream) {
    stream << 999 << std::endl;
  };

  void run(){};

 protected:
  template <typename, typename, typename...>
  friend class DataStream;

  template <typename Input, typename Output, typename... TransformChain>
  explicit DataStream(DataStream<Input, Output, TransformChain...>&& dataStream)
      : ifca(std::move(dataStream.ifca)) {
    LOG_DEBUG() << "DataStream move constructor";
  }

  template <typename Input, typename Output, typename... TransformChain,
            typename Transform>
  explicit DataStream(DataStream<Input, Output, TransformChain...>&& dataStream,
                      Transform&& transform)
      : ifca(std::move(dataStream.ifca), FWD(transform)) {
    LOG_DEBUG() << "DataStream transform constructor";
  }

  template <typename Input, typename Container,
            typename = std::enable_if_t<is_stl_container_v<Container>>>
  decltype(auto) fromContainer(Container&& container) {
    // if In == Container::value_type return this, else new stream
    for (auto&& i = container.begin(); i != container.end(); ++i) {
      //  // if (corked()) {
      //  //   // codition_variable: wait for uncork
      //  // }
      //  // ifca::drain_sfuture drain = ifca.write(*i);
      //  // drain.wait();

      LOG_DEBUG() << *i;
    }
    return *this;
  }

  template <typename Input, typename Stream,
            typename = std::enable_if_t<
                std::is_base_of_v<std::istream, std::decay_t<Stream>>>>
  decltype(auto) fromStream(Stream&& stream) {
    // TODO: if ifstream check eof
    Input a, b;
    stream >> a >> b;
    LOG_DEBUG() << a << b;
    return *this;
  }

  template <typename Transform>
  auto addTransform(Transform&& transform) {
    // TODO: get rid of input change...
    constexpr std::size_t TransformsCount = sizeof...(Transforms);
    if constexpr (TransformsCount > 0)
      return DataStream<In, typename Transform::output_type, Transforms...,
                        Transform>(std::move(ifca), FWD(transform));
    else
      return DataStream<typename Transform::input_type,
                        typename Transform::output_type, Transform>(
          std::move(ifca), FWD(transform));
  };

  void cork(){};
  bool corked() { return false; };
  void uncork(){};

 private:
  ifca::IfcaImpl<In, Out, Transforms...> ifca;
  // std::mutex m;
  // std::condition_variable cv;
  // bool startStreaming;
  // bool endStreaming;
  //  bool corked;
};

}  // namespace stream

#endif  // DATA_STREAM_H
