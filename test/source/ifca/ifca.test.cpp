#include <doctest/doctest.h>

#include <functional>

#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
#include "ifca/isIfcaInterface.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"

namespace ifca {

TEST_CASE("Ifca implementation") {
  std::function filter = [](int& chunk) -> bool {
    LOG_DEBUG() << "Filter called";
    INFO("filter call ", chunk);
    return chunk % 2;
  };
  std::function each = [](int& chunk) {
    LOG_DEBUG() << "Each called";
    INFO("each call ", chunk);
    return chunk + 1;
  };

  Ifca<int> emptyIfca;
  using EmptyIfca_type = Ifca<int>;
  REQUIRE(is_ifca_interface_v<EmptyIfca_type>);
  auto T1 = Each(each);
  using T_type = decltype(Each(each));
  REQUIRE(is_transform_expression_v<T_type>);

  int testValue = 11;
  // int testValue2 = 12;

  SUBCASE("Empty Ifca") {
    emptyIfca.write(testValue);
    auto&& res = emptyIfca.read().get();
    CHECK_EQ(testValue, res);
  }

  SUBCASE("Empty Ifca sum") {
    auto emptyIfcaSum = Ifca<int>() + Each(each);
    emptyIfcaSum.write(testValue);
    auto&& res = emptyIfcaSum.read().get();
    CHECK_EQ(testValue + 1, res);
  }

  // SUBCASE("Single Ifca") {
  //   auto singleIfca =
  //       Ifca<EmptyIfca_type, T_type>(std::move(emptyIfca), std::move(T1));
  //   singleIfca.write(testValue);
  //   auto&& res = singleIfca.read().get();
  //   CHECK_EQ(testValue + 1, res);
  // }

  // SUBCASE("Single Ifca sum") {
  //   auto singleIfca =
  //       Ifca<EmptyIfca_type, T_type>(std::move(emptyIfca), std::move(T1));
  //   auto ifca = std::move(singleIfca) + Each(each);
  //   ifca.write(testValue);
  //   auto&& res = ifca.read().get();
  //   CHECK_EQ(testValue + 2, res);
  // }

  // SUBCASE("Ifca filter") {
  //   auto ifca = Ifca<EmptyIfca_type, decltype(Filter(filter))>(
  //                   std::move(emptyIfca), Filter(filter)) +
  //               std::move(T1);

  //   ifca.write(testValue);
  //   ifca.write(testValue2);
  //   auto&& res = ifca.read().get();
  //   CHECK_EQ(testValue + 1, res);
  // }
}

}  // namespace ifca