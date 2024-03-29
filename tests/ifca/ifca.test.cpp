#include "ifca/ifca.hpp"

#include <doctest/doctest.h>

#include <functional>

#include "../testClass.hpp"
#include "helpers/Logger/logger.hpp"
#include "ifca/isIfcaInterface.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/transform/map.hpp"

namespace ifca {

TEST_CASE("Ifca implementation") {
  auto onlyOdd = [](int& chunk) -> bool {
    INFO("filter call ", chunk);
    return chunk % 2;
  };
  auto incrementByOne = [](int chunk) {
    INFO("each call ", chunk);
    return chunk + 1;
  };

  int testOddValue = 11;
  int testEvenValue = 14;

  SUBCASE("Empty Ifca") {
    auto ifca = Ifca<int>();
    REQUIRE(is_ifca_interface_v<IfcaImpl<int, int>>);
    REQUIRE(is_ifca_interface_v<decltype(ifca)>);

    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue, res);
  }

  SUBCASE("Ifca- adding transform") {
    auto ifca = Ifca().addTransform(map(incrementByOne));
    REQUIRE(is_ifca_interface_v<decltype(ifca)>);
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Ifca- adding multiple transforms") {
    auto ifca = Ifca()
                    .addTransform(map(incrementByOne))
                    .addTransform(map(incrementByOne))
                    .addTransform(map(incrementByOne));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 3, res);
  }

  SUBCASE("Ifca filter") {
    auto ifca =
        Ifca().addTransform(filter(onlyOdd)).addTransform(map(incrementByOne));

    ifca.write(testOddValue);
    ifca.write(testEvenValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  // SUBCASE("Transform returning reference") {
  //   using TestClass = test_utils::TestClass;
  //   auto tc = TestClass("TestClass");
  //   auto& tcRef = tc;
  //   auto passUnchanged = [](TestClass& chunk) -> TestClass& { return chunk;
  //   };

  //  SUBCASE("Empty ifca") {
  //    auto ifca = Ifca<TestClass&>();
  //    ifca.write(tcRef);
  //    TestClass& result = ifca.read().get();
  //    CHECK_EQ(tc.copy_count_, 0);
  //    CHECK_EQ(&tcRef, &result);
  //    CHECK_EQ(&tc, &result);
  //  }

  //  SUBCASE("Single transform ifca") {
  //    auto ifca = Ifca().addTransform(map(passUnchanged));
  //    ifca.write(tcRef);
  //    TestClass& result = ifca.read().get();

  //    CHECK_EQ(tc.copy_count_, 0);
  //    CHECK_EQ(&tcRef, &result);
  //    CHECK_EQ(&tc, &result);
  //  }

  //  SUBCASE("Transforms chain ifca") {
  //    auto ifca = Ifca()
  //                    .addTransform(map(passUnchanged))
  //                    .addTransform(map(passUnchanged));

  //    ifca.write(tcRef);
  //    TestClass& result = ifca.read().get();

  //    CHECK_EQ(tc.copy_count_, 0);
  //    CHECK_EQ(&tcRef, &result);
  //    CHECK_EQ(&tc, &result);
  //  }
  //}

  SUBCASE("Transform as free function") {
    auto ifca = Ifca().addTransform(map(test_utils::freeFunction));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Transform as function pointer") {
    int (*functionPointer)(int) = &test_utils::freeFunction;
    auto ifca = Ifca().addTransform(map(functionPointer));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Transform as function reference") {
    int (&functionReference)(int) = test_utils::freeFunction;
    auto ifca = Ifca().addTransform(map(functionReference));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Transform as rvalue std::function") {
    auto ifca = Ifca().addTransform(
        map(std::function<int(int)>([](int chunk) { return chunk + 1; })));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Transform as lvalue lambda") {
    auto lvalueLambda = [](int chunk) { return chunk + 1; };
    auto ifca = Ifca().addTransform(map(lvalueLambda));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Transform as rvalue lambda") {
    auto ifca = Ifca().addTransform(map([](int chunk) { return chunk + 1; }));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }

  SUBCASE("Transform as functor") {
    struct TestFunctor {
      int operator()(int chunk) { return chunk + 1; }
    };
    auto functor = TestFunctor();
    auto ifca = Ifca().addTransform(map(functor));
    ifca.write(testOddValue);
    auto&& res = ifca.read().get();
    CHECK_EQ(testOddValue + 1, res);
  }
}

}  // namespace ifca