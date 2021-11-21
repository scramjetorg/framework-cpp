#include <doctest/doctest.h>

#include <functional>

#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"

namespace ifca {

TEST_CASE("Ifca implementation") {
  std::function filter = [](int& chunk) {
    LOG_DEBUG() << "filter call " << chunk;
    return chunk % 2;
  };
  std::function each = [](int& chunk) {
    LOG_DEBUG() << "each call " << chunk;
    return chunk + 1;
  };

  auto testValue = 11;

  SUBCASE("Empty Ifca") {
    auto&& emptyIfca = Ifca<>();
    auto&& res = emptyIfca(testValue);
    LOG_INFO() << "Empty Ifca result: " << res;
    CHECK_EQ(testValue, res);
  }

  SUBCASE("Empty Ifca sum") {
    auto&& emptyIfcaSum = Ifca<>() + Each(each);
    auto&& res = emptyIfcaSum(testValue);
    LOG_INFO() << "Empty Ifca sum result: " << res;
    CHECK_EQ(testValue + 1, res);
  }

  SUBCASE("Single Ifca") {
    using eachType = decltype(Each(each));
    auto&& singleIfca = Ifca<eachType>(Each(each));
    auto&& res = singleIfca(testValue);
    LOG_INFO() << "Single Ifca result: " << res;
    CHECK_EQ(testValue + 1, res);
  }

  SUBCASE("Single Ifca sum") {
    using eachType = decltype(Each(each));
    auto&& singleIfcaSum = Ifca<eachType>(Each(each)) + Each(each);
    auto&& res = singleIfcaSum(testValue);
    LOG_INFO() << "Single Ifca sum result: " << res;
    CHECK_EQ(testValue + 2, res);
  }

  SUBCASE("Ifca") {
    using eachType = decltype(Each(each));
    auto&& ifca = Ifca<eachType, eachType>(Each(each), Each(each));
    auto&& res = ifca(testValue);
    LOG_INFO() << "Ifca result: " << res;
    CHECK_EQ(testValue + 2, res);
  }

  SUBCASE("Ifca sum") {
    using eachType = decltype(Each(each));
    auto&& ifca =
        Ifca<eachType, eachType>(Each(each), Each(each)) + Filter(filter);
    auto&& res = ifca(testValue);
    LOG_INFO() << "Ifca result: " << res;
    CHECK_EQ(testValue + 2, res);
  }
}

}  // namespace ifca