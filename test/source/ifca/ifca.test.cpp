#include "ifca/ifca.hpp"

#include <doctest/doctest.h>

#include <functional>

#include "helpers/Logger/logger.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/isIfcaInterface.hpp"

namespace ifca {

TEST_CASE("Ifca implementation") {
  std::function filter = [](int& chunk) -> int {
    INFO("filter call ", chunk);
    return chunk % 2;
  };
  std::function each = [](int& chunk) {
    INFO("each call ", chunk);
    return chunk + 1;
  };
  auto T1 = Each(each);
  auto T2 = Each(each);
  auto T3 = Each(each);
  auto T4 = Each(each);
  using T_type = decltype(Each(each));

  SUBCASE("Ifca tuple") {
    Ifca<int, int> emptyIfca;
    int chunk = 5;
    int res = emptyIfca(chunk);
    CHECK_EQ(chunk, res);

    Ifca<decltype(emptyIfca), T_type> ifca(std::move(emptyIfca),
                                            std::move(T1));

    Ifca<decltype(ifca), T_type> ifca2(std::move(ifca), std::move(T2));
    Ifca<decltype(ifca2), T_type> ifca3(std::move(ifca2), std::move(T3));
    CHECK_EQ(chunk + 3, ifca3(chunk));

    auto ifca4 = std::move(ifca3) + std::move(T4);
    CHECK_EQ(chunk + 4, ifca4(chunk));

    auto ifca5 = std::move(ifca4) + Each(each);
    CHECK_EQ(chunk + 5, ifca5(chunk));
  }

  // FIXME: why it crashes when auto- check behaviour for reference values
  int testValue = 11;
  // int testValue2 = 12;

  SUBCASE("Empty Ifca") {
    auto emptyIfca = Ifca<int, int>();
    REQUIRE(is_ifca_interface_v<decltype(emptyIfca)>);
    emptyIfca.write(testValue);
    auto&& res = emptyIfca.read().get();
    CHECK_EQ(testValue, res);
  }

  // SUBCASE("Empty Ifca sum") {
  //   auto emptyIfcaSum = Ifca<int, int>() + Each(each);
  //   emptyIfcaSum.write(testValue);
  //   auto&& res = emptyIfcaSum.read().get();
  //   LOG_INFO() << "Empty Ifca sum result: " << res;
  //   CHECK_EQ(testValue + 1, res);
  // }

  // SUBCASE("Single Ifca") {
  //   using eachType = decltype(Each(each));
  //   LOG_ERROR() << "SINGLE " << IsTransformExpression<eachType>::value
  //               << is_crtp_interface_of<TransformExpression,
  //               eachType>::value;
  //   auto singleIfca = Ifca<eachType>(Each(each));
  //   singleIfca.write(testValue);
  //   auto&& res = singleIfca.read().get();
  //   LOG_INFO() << "Single Ifca result: " << res;
  //   CHECK_EQ(testValue + 1, res);
  // }

  // SUBCASE("Single Ifca sum") {
  //   using eachType = decltype(Each(each));
  //   auto&& singleIfcaSum = Ifca<eachType>(Each(each)) + Each(each);
  //   singleIfcaSum.write(testValue);
  //   auto&& res = singleIfcaSum.read().get();
  //   LOG_INFO() << "Single Ifca sum result: " << res;
  //   CHECK_EQ(testValue + 2, res);
  // }

  // SUBCASE("Ifca") {
  //   using eachType = decltype(Each(each));
  //   auto ifca = Ifca<eachType, eachType, bool>(Each(each), Each(each));

  //   ifca.write(testValue);
  //   auto&& res = ifca.read().get();
  //   LOG_INFO() << "Ifca result: " << res;
  //   CHECK_EQ(testValue + 2, res);
  // }

  SUBCASE("Ifca sum") {
    // using eachType = decltype(Each(each));
    // auto ifca =
    //     Ifca<eachType, eachType, bool>(Each(each), Each(each)) + Each(each);
    // auto ifca = Ifca<eachType, eachType, bool>(Each(each), Each(each));

    // auto f = Each(each);
    // Ifca<Ifca<eachType, eachType, bool>, eachType, bool>(
    //     Ifca<eachType, eachType, bool>(Each(each), Each(each)), Each(each));
    // LOG_ERROR() << "IFCA CHECK: " << typeid(ifca).name();
    // LOG_ERROR() << "Ifca Value: " << isIfcaInterface<decltype(ifca)>::value;
    // << std::is_same_v<
    //        IfcaMethods<
    //            typename Ifca<eachType, eachType, bool>::Impl,
    //            typename Ifca<eachType, eachType, bool>::input_type,
    //            typename Ifca<eachType, eachType, bool>::result_type>,
    //        typename Ifca<eachType, eachType, bool>::
    //            base_type> <<
    // LOG_ERROR() << std::is_base_of<
    //     IfcaMethods<Ifca<eachType, eachType, bool>,
    //                 typename Ifca<eachType, eachType, bool>::input_type,
    //                 typename Ifca<eachType, eachType, bool>::result_type>,
    //     Ifca<eachType, eachType, bool>>::value;

    // REQUIRE(isIfcaInterface<decltype(ifca)>::value);

    // ifca.write(testValue);
    // auto&& res = ifca.read().get();
    // LOG_INFO() << "Ifca result: " << res;
    // CHECK_EQ(testValue + 3, res);
  }

  // SUBCASE("Ifca filter") {
  //   using eachType = decltype(Each(each));
  //   auto&& ifca =
  //       Ifca<eachType, eachType, bool>(Each(each), Each(each)) +
  //       Filter(filter);
  //   ifca.write(testValue);
  //   ifca.write(testValue2);
  //   auto&& res = ifca.read().get();
  //   LOG_INFO() << "Ifca result: " << res;
  //   CHECK_EQ(testValue + 2, res);
  // }
}

}  // namespace ifca