#include <doctest/doctest.h>

#include <functional>
#include <string>

#include "../../testClass.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/isTransformExpression.hpp"

namespace ifca {

TEST_CASE("EachTransform") {
  auto modified = false;
  auto stoiAndIncrement = [&modified](std::string& chunk) {
    modified = true;
    return std::stoi(chunk) + 1;
  };
  auto rejectedFunc = [] {};

  std::string testValue = "10";
  std::string expectedResult = "10";
  std::string result;
  auto resolvedFunc = [&result](std::string& chunk) { result = chunk; };
  auto eachTransform = each(stoiAndIncrement);
  REQUIRE(is_transform_expression_v<decltype(eachTransform)>);

  SUBCASE("Chunk resolved") {
    eachTransform(testValue, resolvedFunc, rejectedFunc);
    CHECK(modified);
    CHECK_EQ(result, expectedResult);
  }

  SUBCASE("Chunk passed to next transform") {
    auto nextPassed = false;
    auto nextTransform = [&nextPassed](std::string& chunk,
                                       decltype(resolvedFunc) resolve,
                                       decltype(rejectedFunc)) {
      nextPassed = true;
      resolve(chunk);
    };
    eachTransform(testValue, resolvedFunc, rejectedFunc, nextTransform);
    CHECK(nextPassed);
    CHECK(modified);
    CHECK_EQ(result, expectedResult);
  }
}

TEST_CASE("EachTransform no implicit copies") {
  using TestClass = test_utils::TestClass;

  auto passUnchanged = [](TestClass& chunk) -> TestClass& {
    CHECK_EQ(chunk.copy_count_, 0);
    return chunk;
  };
  auto resolvedFunc = [](TestClass& chunk) { CHECK_EQ(chunk.copy_count_, 0); };
  auto rejectedFunc = [] {};
  auto eachTransform = each(passUnchanged);

  auto testValue = TestClass();
  eachTransform(testValue, resolvedFunc, rejectedFunc);
}

}  // namespace ifca