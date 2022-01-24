#include <doctest/doctest.h>

#include "streams/dataStream.hpp"

// #include <functional>

#include "helpers/Logger/logger.hpp"

namespace stream {

TEST_CASE("DataStream implementation") {
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