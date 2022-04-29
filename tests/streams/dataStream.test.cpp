#include "streams/dataStream.hpp"

#include <doctest/doctest.h>

// #include <functional>
#include <sstream>
#include <string>

//#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/each.hpp"

namespace stream {

TEST_CASE("DataStream implementation") {
  // auto stream = DataStream<double>(4);

  auto incrementByOne = [](int chunk) -> int {
    INFO("each call ", chunk);
    return chunk + 1;
  };

  // auto ifca = ifca::Ifca().addTransform(ifca::each(incrementByOne));
  // auto streamIfca = DataStream<int, int,
  // decltype(ifca::each(incrementByOne))>(
  //    ifca::Ifca(), ifca::each(incrementByOne));

  // DataStream<double>(4).write(3);
  // DataStream<>().each([](int a) -> int { return a; }).map(incrementByOne);

  SUBCASE("read from container") {
    std::vector<int> input = {7, 5, 16, 8};
    std::vector<int> output;
    DataStream<>().from(input);
  }

  SUBCASE("write to container") {
    std::vector<int> input = {7, 5, 16, 8};
    std::vector<int> output;
    DataStream<>().from(input).toContainer(output);
    for (auto& v : output) {
      LOG_WARNING() << "container: " << v;
    }
  } 

  SUBCASE("read from stream") {
    std::stringstream ss;
    ss << 100 << 200;
    DataStream<>().from<int>(ss);
  }

  SUBCASE("write to stream") {
    std::stringstream ss;
    ss << 100 << 200;
    DataStream<>().from<int>(ss).toOutStream(std::cout);
  }

  SUBCASE("write to array ") {
    std::vector<int> input = {7, 5, 16, 8};
    int array[4];
    DataStream<>().from(input).toArray(array);
    LOG_WARNING() << "Array: " << array[0] << array[1] << array[2] << array[3];
  }

  SUBCASE("each() method") {}
  SUBCASE("map() method") {}
  SUBCASE("filter() method") {}
  SUBCASE("pipe() method") {}
  SUBCASE("reduce() method") {}
  SUBCASE("toArray() method") {}
  SUBCASE("toContainer() method") {}
  SUBCASE("toOutStream() method") {}
  SUBCASE("run() method") {}
  // auto passOddNumber = [](int& chunk) { return chunk % 2; };
  // auto incrementByOne = [](int& chunk) { return chunk + 1; };

  // auto evenNumber = 10;
  // auto oddNumber = 13;

  //   SUBCASE("DataStream build") {
  //     auto stream = DataStream();
  //     auto res = stream(testValue);
  //     LOG_INFO() << "IFCA result: " << res;
  //     CHECK_EQ(testValue, res);
  //   }

  // SUBCASE("DataStream single transform") {
  //   auto stream = DataStream().each(each);
  //   auto res = stream(testValue);
  //   LOG_INFO() << "IFCA result: " << res;
  //   CHECK_EQ(testValue + 1, res);
  // }

  // SUBCASE("DataStream transforms") {
  //   auto stream =
  //   DataStream().each(each).each(each).filter(filter).each(each); auto res
  // =
  //   stream(testValue); LOG_INFO() << "IFCA result: " << res;
  //   CHECK_EQ(testValue + 3, res);
  // }
}

}  // namespace stream