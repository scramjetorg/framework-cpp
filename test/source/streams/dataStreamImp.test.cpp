// #include <doctest/doctest.h>

// #include <functional>

// #include "helpers/Logger/logger.hpp"
// #include "streams/dataStream.hpp"

// namespace stream {

// TEST_CASE("DataStream implementation") {
//   std::function filter = [](int& chunk) {
//     LOG_DEBUG() << "filter call " << chunk;
//     return chunk % 2;
//   };
//   std::function each = [](int& chunk) {
//     LOG_DEBUG() << "each call " << chunk;
//     return chunk + 1;
//   };

//   auto testValue = 11;

//   SUBCASE("DataStream build") {
//     auto stream = DataStream();
//     auto res = stream(testValue);
//     LOG_INFO() << "IFCA result: " << res;
//     CHECK_EQ(testValue, res);
//   }

//   // SUBCASE("DataStream single transform") {
//   //   auto stream = DataStream().each(each);
//   //   auto res = stream(testValue);
//   //   LOG_INFO() << "IFCA result: " << res;
//   //   CHECK_EQ(testValue + 1, res);
//   // }

//   // SUBCASE("DataStream transforms") {
//   //   auto stream =
//   //   DataStream().each(each).each(each).filter(filter).each(each); auto res
//   =
//   //   stream(testValue); LOG_INFO() << "IFCA result: " << res;
//   //   CHECK_EQ(testValue + 3, res);
//   // }
// }

// }  // namespace stream