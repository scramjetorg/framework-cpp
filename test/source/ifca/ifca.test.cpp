#include <doctest/doctest.h>

#include <functional>

#include "../testClass.hpp"
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

  SUBCASE("Transform returning reference") {
    using TestClass = test_utils::TestClass;
    auto tc = TestClass("TestClass");
    auto& tcRef = tc;
    std::function<TestClass&(TestClass&)> passUnchanged =
        [](TestClass& chunk) -> TestClass& { return chunk; };

    SUBCASE("Empty ifca") {
      auto ifca = Ifca<TestClass&>();
      ifca.write(tcRef);
      TestClass& result = ifca.read().get();
      CHECK_EQ(tc.copy_count_, 0);
      CHECK_EQ(&tcRef, &result);
      CHECK_EQ(&tc, &result);
    }

    SUBCASE("Single transform ifca") {
      auto ifca = Ifca(each(passUnchanged));
      ifca.write(tcRef);
      TestClass& result = ifca.read().get();

      CHECK_EQ(tc.copy_count_, 0);
      CHECK_EQ(&tcRef, &result);
      CHECK_EQ(&tc, &result);
    }

    SUBCASE("Transforms chain ifca") {
      auto ifca = Ifca(each(passUnchanged)) + each(passUnchanged);

      ifca.write(tcRef);
      TestClass& result = ifca.read().get();

      CHECK_EQ(tc.copy_count_, 0);
      CHECK_EQ(&tcRef, &result);
      CHECK_EQ(&tc, &result);
    }
  }
}

}  // namespace ifca