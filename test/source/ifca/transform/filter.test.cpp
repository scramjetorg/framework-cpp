#include <doctest/doctest.h>

#include <functional>

#include "../../testClass.hpp"
#include "ifca/transform/filter.hpp"

namespace ifca {

TEST_CASE("FilterTransform") {
  auto resolved = false;
  auto rejected = false;
  std::function<void()> rejectedFunc = [&rejected]() { rejected = true; };

  SUBCASE("Without deleter") {
    std::function<bool(int)> filterFunc = [](int chunk) {
      return !(chunk % 2);
    };
    std::function<void(int)> resolvedFunc = [&resolved](int) {
      resolved = true;
    };

    auto filter = Filter(filterFunc);

    SUBCASE("Chunk resolved") {
      filter(0, resolvedFunc, rejectedFunc);
      CHECK_FALSE(rejected);
      CHECK(resolved);
    }

    SUBCASE("Chunk rejected") {
      filter(1, resolvedFunc, rejectedFunc);
      CHECK(rejected);
      CHECK_FALSE(resolved);
    }

    SUBCASE("Chunk passed to next transform") {
      auto nextPassed = false;
      auto nextTransform = [&nextPassed](int chunk,
                                         decltype(resolvedFunc) resolve,
                                         decltype(rejectedFunc)) {
        nextPassed = true;
        resolve(chunk);
      };

      filter(0, resolvedFunc, rejectedFunc, nextTransform);
      CHECK_FALSE(rejected);
      CHECK(nextPassed);
      CHECK(resolved);
    }
  }

  SUBCASE("With deleter") {
    std::function<bool(test_utils::TestClass*)> filterFunc =
        [](test_utils::TestClass* chunk) { return (chunk->name_ == ""); };

    std::function<void(test_utils::TestClass*)> resolvedFunc =
        [&resolved](test_utils::TestClass*) { resolved = true; };

    auto deleted = false;
    std::function<void(test_utils::TestClass*)> deleterFunc =
        [&deleted](test_utils::TestClass* chunk) {
          deleted = true;
          delete chunk;
        };

    auto filter = Filter(filterFunc, deleterFunc);

    SUBCASE("Chunk resolved") {
      test_utils::TestClass* tcPass = new test_utils::TestClass();
      filter(tcPass, resolvedFunc, rejectedFunc);
      CHECK_FALSE(rejected);
      CHECK(resolved);
      delete tcPass;
    }

    SUBCASE("Chunk rejected") {
      test_utils::TestClass* tcReject = new test_utils::TestClass("reject");
      filter(tcReject, resolvedFunc, rejectedFunc);
      CHECK(rejected);
      CHECK_FALSE(resolved);
      CHECK(deleted);
    }

    SUBCASE("Chunk passed to next transform") {
      test_utils::TestClass* tcPass = new test_utils::TestClass();
      auto nextPassed = false;
      auto nextTransform = [&nextPassed](test_utils::TestClass* chunk,
                                         decltype(resolvedFunc) resolve,
                                         decltype(rejectedFunc)) {
        nextPassed = true;
        resolve(chunk);
      };
      filter(tcPass, resolvedFunc, rejectedFunc, nextTransform);
      CHECK_FALSE(rejected);
      CHECK(nextPassed);
      CHECK(resolved);
      delete tcPass;
    }
  }
}

TEST_CASE("FilterTransform no implicit copies") {
  using TestClass = test_utils::TestClass;

  std::function<bool(TestClass&)> filterPassAll = [](TestClass& chunk) -> bool {
    CHECK_EQ(chunk.copy_count_, 0);
    return true;
  };
  std::function<bool(TestClass&)> filterPassNone =
      [](TestClass& chunk) -> bool {
    CHECK_EQ(chunk.copy_count_, 0);
    return false;
  };

  std::function<void(TestClass&)> resolvedFunc = [](TestClass& chunk) {
    CHECK_EQ(chunk.copy_count_, 0);
  };
  auto rejectedFunc = [] {};
  auto testValue = TestClass();

  auto filterAll = Filter(filterPassAll);
  filterAll(testValue, resolvedFunc, rejectedFunc);
  CHECK_EQ(testValue.copy_count_, 0);

  auto filterNone = Filter(filterPassNone);
  filterNone(testValue, resolvedFunc, rejectedFunc);
  CHECK_EQ(testValue.copy_count_, 0);
}

}  // namespace ifca