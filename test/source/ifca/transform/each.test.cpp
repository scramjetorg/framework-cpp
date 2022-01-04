#include <doctest/doctest.h>

#include <functional>
#include <string>

#include "../../testClass.hpp"
#include "ifca/transform/each.hpp"

namespace ifca {

TEST_CASE("EachTransform") {
  auto modified = false;
  std::function<int(std::string&)> eachFunc = [&modified](std::string& chunk) {
    modified = true;
    return std::stoi(chunk) + 1;
  };
  auto rejectedFunc = [] {};

  std::string testValue = "10";
  int expectedResult = 11;
  int result = 0;
  std::function<void(int)> resolvedFunc = [&result](int chunk) {
    result = chunk;
  };
  auto each = Each(eachFunc);

  SUBCASE("Chunk resolved") {
    each(testValue, resolvedFunc, rejectedFunc);
    CHECK(modified);
    CHECK_EQ(result, expectedResult);
  }

  SUBCASE("Chunk passed to next transform") {
    auto nextPassed = false;
    auto nextTransform = [&nextPassed](int chunk,
                                       decltype(resolvedFunc) resolve,
                                       decltype(rejectedFunc)) {
      nextPassed = true;
      resolve(chunk);
    };
    each(testValue, resolvedFunc, rejectedFunc, nextTransform);
    CHECK(nextPassed);
    CHECK(modified);
    CHECK_EQ(result, expectedResult);
  }
}

TEST_CASE("EachTransform no implicit copies") {
  using TestClass = test_utils::TestClass;

  std::function<TestClass&(TestClass&)> eachFunc =
      [](TestClass& chunk) -> TestClass& {
    CHECK_EQ(chunk.copy_count_, 0);
    return chunk;
  };
  std::function<void(TestClass&)> resolvedFunc = [](TestClass& chunk) {
    CHECK_EQ(chunk.copy_count_, 0);
  };
  auto rejectedFunc = [] {};
  auto each = Each(eachFunc);

  auto testValue = TestClass();
  each(testValue, resolvedFunc, rejectedFunc);
}

}  // namespace ifca