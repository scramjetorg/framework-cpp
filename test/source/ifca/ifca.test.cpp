#include <doctest/doctest.h>

#include <functional>

#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
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

  int testOddValue = 11;
  int testEvenValue = 14;

  SUBCASE("Empty Ifca") {
    auto ifca = Ifca<int>();
    REQUIRE(is_ifca_interface_v<decltype(ifca)>);

    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue, res);
  }

  SUBCASE("Empty Ifca- adding transform") {
    auto ifca = Ifca<int>().addTransform(each(incrementByOne));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Ifca with single transform") {
    auto ifca = Ifca(each(incrementByOne));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Adding transform to ifca with single transform") {
    auto ifca = Ifca(each(incrementByOne)).addTransform(each(incrementByOne));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 2, res);
  }

  SUBCASE("Adding transform to ifca chain") {
    auto ifca = Ifca(each(incrementByOne))
                    .addTransform(each(incrementByOne))
                    .addTransform(each(incrementByOne));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 3, res);
  }

  SUBCASE("Ifca filter") {
    auto ifca = Ifca(filter(onlyOdd)).addTransform(each(incrementByOne));

    ifca.write(testOddValue);
    ifca.write(testEvenValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }
}

}  // namespace ifca