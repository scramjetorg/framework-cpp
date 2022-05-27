#ifndef DATA_STREAM_H
#define DATA_STREAM_H

#include <iterator>
#include <optional>
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

namespace stream {

template <typename In, typename Out, typename... Transforms>
class DataStream {
 public:
  DataStream(unsigned int max_parallel = maxParallel())
      : ifca(max_parallel),
        inputReady(false),
        outputReady(false),
        streamingStarted(false),
        // streamingFinished(std::make_shared<bool>(false)),
        m(std::make_unique<std::mutex>()),
        condVar(std::make_unique<std::condition_variable>()),
        corked(false) {
    LOG_DEBUG() << "DataStream deafult constructor";
  };

  template <typename Input, typename Output, typename... TransformChain>
  DataStream(DataStream<Input, Output, TransformChain...>&& dataStream)
      : ifca(std::move(dataStream.ifca)),
        inputReady(std::move(dataStream.inputReady)),
        outputReady(std::move(dataStream.outputReady)),
        streamingStarted(std::move(dataStream.streamingStarted)),
        // streamingFinished(dataStream.streamingFinished),
        inputReader(std::move(dataStream.inputReader)),
        outputWriter(std::move(dataStream.outputWriter)),
        m(std::move(dataStream.m)),
        condVar(std::move(dataStream.condVar)) {
    LOG_DEBUG() << "DataStream move constructor";
    dataStream.streamingFinished.reset();
  }

  ~DataStream() {
    LOG_DEBUG() << "DataStream destructor" << inputReader.valid();
    if (inputReader.valid()) {
      // using namespace std::chrono_literals;
      // std::this_thread::sleep_for(2000ms);
      // endStreaming();
      inputReader.wait();
      if (!ifca.ended()) ifca.end();
    }
    LOG_DEBUG() << "Streaming ended";
  };

  template <typename Source,
            typename = std::enable_if_t<!std::is_same_v<In, void> &&
                                        is_stl_container_v<Source>>>
  decltype(auto) from(Source&& source) {
    // static_assert(std::is_same_v<In, void>, "void");
    using Input = typename std::decay_t<Source>::value_type;
    /* if constexpr (!std::is_same_v<Input, In>) {
       LOG_DEBUG() << "Other In";
       return DataStream<Input, Out, Transforms...>(std::move(*this))
           .from<Source>(FWD(source));
     } else {*/
    fromContainer<Input>(FWD(source));
    return *this;
    //}
  }

  template <typename Input = In, typename Source,
            typename = std::enable_if_t<
                std::is_base_of_v<std::istream, std::decay_t<Source>>>>
  decltype(auto) from(Source&& source) {
    // if constexpr (!std::is_same_v<Input, In>) {
    //   LOG_DEBUG() << "Other In";
    //   return DataStream<Input, Out, Transforms...>(std::move(*this))
    //       .from<Input>(FWD(source));
    // } else {
    fromStream<Input>(FWD(source));
    return *this;
    //}
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
  }
  template <typename Function>
  auto map(Function&& function) {
    return addTransform(ifca::map(FWD(function)));
  }
  template <typename Function>
  auto filter(Function&& function) {
    return addTransform(ifca::filter(FWD(function)));
  }
  auto pipe(){};
  template <typename Function>
  auto reduce(Function&& function) {
    return addTransform(ifca::reduce(FWD(function)));
  }

  // Output methods
  template <typename T, size_t size>
  void toArray(T (&array)[size]) {
    auto it = 0;
    while (it < size) {
      array[it++] = it;
    }
  }

  template <typename Container,
            typename = std::enable_if_t<!std::is_same_v<In, void> &&
                                        is_stl_container_v<Container>>>
  decltype(auto) toContainer(Container&& container) {
    outputWriter = std::async(std::launch::async, [&, this]() -> void {
      LOG_DEBUG() << "<--------outputWriter";
      while (!ifca.ended()) {
        //using value_type = typename std::decay_t<Container>::value_type;
        auto&& value = ifca.read().get();
        LOG_DEBUG() << "outputWriter val: " << value;
        container.insert(container.cend(), value);
      }
    });

    outputReady = true;
    checkStreamState();
    return *this;
  }

  template <typename Stream, typename = std::enable_if_t<std::is_base_of_v<
                                 std::ostream, std::decay_t<Stream>>>>
  void toOutStream(Stream&& stream) {
    stream << 999 << std::endl;
  }

  void run() { LOG_DEBUG() << "run()"; };

 protected:
  template <typename, typename, typename...>
  friend class DataStream;

  template <typename Input, typename Output, typename... TransformChain,
            typename Transform>
  explicit DataStream(DataStream<Input, Output, TransformChain...>&& dataStream,
                      Transform&& transform)
      : ifca(std::move(dataStream.ifca), FWD(transform)),
        inputReady(std::move(dataStream.inputReady)),
        outputReady(std::move(dataStream.outputReady)),
        streamingStarted(std::move(dataStream.streamingStarted)),
        // streamingFinished(std::move(dataStream.streamingFinished)),
        inputReader(std::move(dataStream.inputReader)),
        outputWriter(std::move(dataStream.outputWriter)),
        m(std::move(dataStream.m)),
        condVar(std::move(dataStream.condVar)) {
    LOG_DEBUG() << "DataStream transform constructor";
  }

  template <typename Input, typename Container,
            typename = std::enable_if_t<is_stl_container_v<Container>>>
  decltype(auto) fromContainer(Container&& container) {
    inputReader = std::async(std::launch::async, [&, this]() -> void {
      LOG_DEBUG() << "<--------Input thread waitStartStreaming";
      // auto finished = streamingFinished;
      // waitStartStreaming();
      LOG_DEBUG() << "<--------Input thread";
      for (auto&& it = container.begin(); it != container.end(); it++) {
        // LOG_DEBUG() << "dist: " << std::distance(container.begin(), it);
        // waitIfCorked();
        // if (*streamingFinished.get()) break;
        auto drain = ifca.write(FWD(*it));
        drain.wait();
        LOG_DEBUG() << *it;
      }
      // if (!ifca.ended()) ifca.end();
    });
    inputReady = true;
    return *this;
  }

  template <typename Input, typename Stream,
            typename = std::enable_if_t<
                std::is_base_of_v<std::istream, std::decay_t<Stream>>>>
  decltype(auto) fromStream(Stream&& stream) {
    // TODO: if ifstream check eof
    Input a;
    stream >> a;
    LOG_DEBUG() << a;

    // inputReady = true;
    return *this;
  }

  void cork() {
    std::lock_guard<std::mutex> lck(*m);
    corked = true;
    condVar->notify_all();
  };
  void uncork() {
    std::lock_guard<std::mutex> lck(*m);
    corked = false;
    condVar->notify_all();
  };

  void startWriting() {
    static_assert(!std::is_same_v<In, void> && !std::is_same_v<Out, void>);
    // if constexpr (is_stl_container_v<Src>)
    //   inputReader = std::async(std::launch::async, [&, this]() -> void {
    //     // waitStartStreaming();
    //     LOG_DEBUG() << "<--------Input thread";
    //     auto&& container = input.value();
    //     for (auto&& it = container.begin();
    //          it != container.end() || !streamingFinished; ++it) {
    //       waitIfCorked();
    //       // ifca::drain_sfuture drain = ifca.write(FWD(*it));
    //       //  drain.wait();
    //       LOG_DEBUG() << *it;
    //     }
    //     ifca.end();
    //   });

    /*std::lock_guard<std::mutex> lck(*m);
    streamingStarted = true;
    condVar->notify_all();*/
  };

  void startStreaming() {
    LOG_DEBUG() << "startStreaming()";
    std::lock_guard<std::mutex> lck(*m);
    streamingStarted = true;
    condVar->notify_all();
  };

  // void endStreaming() {
  //   LOG_DEBUG() << "endStreaming()";
  //   std::lock_guard<std::mutex> lck(*m);
  //   *streamingFinished = true;
  //   condVar->notify_all();
  // };

  void checkStreamState() {
    LOG_DEBUG() << "checkStreamState()" << inputReady << outputReady;
    if (!inputReady && !outputReady) return;
    startStreaming();
  }

 private:
  // void waitStartStreaming() {
  //   std::unique_lock<std::mutex> lck(*m);
  //   condVar->wait(lck, [&] { return streamingStarted || *streamingFinished;
  //   });
  // }
  // void waitIfCorked() {
  //   if (!corked) return;
  //   std::unique_lock<std::mutex> lck(*m);
  //   condVar->wait(lck, [&] { return !corked || *streamingFinished; });
  // }

  ifca::IfcaImpl<In, Out, Transforms...> ifca;
  bool inputReady;
  bool outputReady;
  bool streamingStarted;
  // std::shared_ptr<bool> streamingFinished;
  std::future<void> inputReader;
  std::future<void> outputWriter;
  std::unique_ptr<std::mutex> m;
  std::unique_ptr<std::condition_variable> condVar;
  bool corked;
};

}  // namespace stream

#endif  // DATA_STREAM_H
