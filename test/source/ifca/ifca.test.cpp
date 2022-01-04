#include "ifca/ifca.hpp"

#include <doctest/doctest.h>

#include <functional>

#include "helpers/Logger/logger.hpp"
#include "ifca/isIfcaInterface.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"

namespace ifca {

TEST_CASE("Ifca implementation") {
  std::function onlyOdd = [](int& chunk) -> bool {
    INFO("filter call ", chunk);
    return chunk % 2;
  };
  std::function incrementByOne = [](int chunk) {
    INFO("each call ", chunk);
    return chunk + 1;
  };

  Ifca<int, int> emptyIfca;
  using EmptyIfca_type = Ifca<int, int>;
  REQUIRE(is_ifca_interface_v<EmptyIfca_type>);
  auto T1 = Each(incrementByOne);
  using T_type = decltype(Each(incrementByOne));
  REQUIRE(is_transform_expression_v<T_type>);

  int testOddValue = 11;
  int testEvenValue = 12;

  SUBCASE("Empty Ifca") {
    emptyIfca.write(testOddValue);
    auto&& res = emptyIfca.read().get();
    CHECK_EQ(testOddValue, res);
  }

  SUBCASE("Empty Ifca sum") {
    auto emptyIfcaSum = Ifca<int, int>() + Each(incrementByOne);
    emptyIfcaSum.write(testOddValue);
    auto&& res = emptyIfcaSum.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Single Ifca") {
    auto singleIfca = Ifca<T_type, void>(std::move(T1));
    singleIfca.write(testOddValue);
    auto&& res = singleIfca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Single Ifca sum") {
    auto singleIfca = Ifca<T_type, void>(std::move(T1));
    auto ifca = std::move(singleIfca) + Each(incrementByOne);
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 2, res);
  }

  SUBCASE("Ifca filter") {
    auto ifca =
        Ifca<decltype(Filter(onlyOdd)), void>(Filter(onlyOdd)) + std::move(T1);

    ifca.write(testOddValue);
    ifca.write(testEvenValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }
}

}  // namespace ifca