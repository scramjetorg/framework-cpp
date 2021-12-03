#include "ifca/ifca.hpp"

#include <doctest/doctest.h>

#include <functional>

#include "helpers/Logger/logger.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"

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
  using T1_type = decltype(T1);

  SUBCASE("Ifca tuple") {
    Ifca<int, int> emptyIfca;
    int chunk = 5;
    int res = emptyIfca(chunk);
    CHECK_EQ(chunk, res);

    Ifca<decltype(emptyIfca), T1_type> ifca(std::move(emptyIfca),
                                            std::move(T1));

    Ifca<decltype(ifca), T1_type> ifca2(std::move(ifca), std::move(T2));
    Ifca<decltype(ifca2), T1_type> ifca3(std::move(ifca2), std::move(T3));
    int ifcaRes = ifca3(chunk);
    CHECK_EQ(chunk + 3, ifcaRes);

    RunTransformChain<decltype(ifca3.transforms_)>(ifca3.transforms_);

    LOG_ERROR()
        << "Unfold 0: "
        << unfoldTransforms<decltype(ifca3.transforms_), decltype(chunk), 0>()(
               ifca3.transforms_, chunk);
    LOG_ERROR() << "Unfold: " <<
        unfoldTransforms<decltype(ifca3.transforms_), decltype(chunk),
                         std::tuple_size_v<decltype(ifca3.transforms_)> - 1U>()(
            ifca3.transforms_, chunk);
    // emptyIfca + T1;  // TODO: check why not working with lvalue
    // emptyIfca + Each(each);

    // using emptyTuple = transform_chain<>::type;
    // // LOG_WARNING() << std::get<0>(tup0);
    // using firstTuple = transform_chain<int, emptyTuple>::type;
    // firstTuple tup = {1};
    // LOG_WARNING() << std::get<0>(tup);
    // using secondTuple = transform_chain<double, firstTuple>::type;
    // secondTuple tup2 = {5, 2.20};
    // LOG_WARNING() << std::get<0>(tup2);
    // LOG_WARNING() << std::get<1>(tup2);

    // struct testClass {
    //   testClass() { LOG_DEBUG() << "TestClass created"; }
    //   testClass(const testClass&) { LOG_DEBUG() << "TestClass copied"; }
    //   testClass& operator=(const testClass&) {
    //     LOG_DEBUG() << "TestClass copied=";
    //     return *this;
    //   }
    //   testClass(testClass&&) { LOG_DEBUG() << "TestClass moved"; }
    //   testClass& operator=(testClass&&) {
    //     LOG_DEBUG() << "TestClass moved=";
    //     return *this;
    //   }
    //   ~testClass() { LOG_DEBUG() << "TestClass deleted"; }
    // };
    // std::tuple<int, int, testClass> t;
    // auto t1 = std::make_tuple(1);
    // {
    //   auto t2 = std::make_tuple(2);
    //   auto t3 = std::make_tuple(testClass());
    //   LOG_WARNING() << "CAT";
    //   t = std::tuple_cat(t1, t2, t3);
    //   LOG_INFO() << std::get<0>(t1);
    //   LOG_INFO() << std::get<0>(t2);
    // }
    // LOG_INFO() << std::get<1>(t);
    // LOG_INFO() << std::get<0>(t1);

    // auto chain = MoveToTransfomChain<std::tuple<int>, int>(std::move(t1), 1);
    // LOG_INFO() << std::get<0>(chain) << std::get<1>(chain);
    LOG_WARNING() << "This is the end";
    // CHECK_EQ(nexIfca, 0);
  }

  // FIXME: why it crashes when auto- check behaviour for reference values
  // int testValue = 11;
  // int testValue2 = 12;

  // SUBCASE("Empty Ifca") {
  //   auto emptyIfca = Ifca<int, int>();
  //   REQUIRE(isIfcaInterface<decltype(emptyIfca)>::value);
  //   emptyIfca.write(testValue);
  //   auto&& res = emptyIfca.read().get();
  //   LOG_INFO() << "Empty Ifca result: " << res;
  //   CHECK_EQ(testValue, res);
  // }

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