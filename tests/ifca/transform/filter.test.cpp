#include <doctest/doctest.h>

#include <functional>

#include "../../testClass.hpp"
#include "ifca/transform/filter.hpp"

namespace ifca {

TEST_CASE("FilterTransform") {
  auto resolved = false;
  auto rejected = false;
  auto rejectedFunc = [&rejected]() { rejected = true; };

  SUBCASE("Without deleter") {
    auto passEven = [](int chunk) { return !(chunk % 2); };
    auto resolvedFunc = [&resolved](int) { resolved = true; };

    auto filterTransform = filter(passEven);

    SUBCASE("Chunk resolved") {
      filterTransform(0, resolvedFunc, rejectedFunc);
      CHECK_FALSE(rejected);
      CHECK(resolved);
    }

    SUBCASE("Chunk rejected") {
      filterTransform(1, resolvedFunc, rejectedFunc);
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

      filterTransform(0, resolvedFunc, rejectedFunc, nextTransform);
      CHECK_FALSE(rejected);
      CHECK(nextPassed);
      CHECK(resolved);
    }
  }

  SUBCASE("With deleter") {
    auto passEmptyName = [](test_utils::TestClass* chunk) {
      return (chunk->name_ == "");
    };

    auto resolvedFunc = [&resolved](test_utils::TestClass*) {
      resolved = true;
    };

    auto deleted = false;
    auto deleterFunc = [&deleted](test_utils::TestClass* chunk) {
      deleted = true;
      delete chunk;
    };

    auto filterTransform = filter(passEmptyName, deleterFunc);

    SUBCASE("Chunk resolved") {
      test_utils::TestClass* tcPass = new test_utils::TestClass();
      filterTransform(tcPass, resolvedFunc, rejectedFunc);
      CHECK_FALSE(rejected);
      CHECK(resolved);
      delete tcPass;
    }

    SUBCASE("Chunk rejected") {
      test_utils::TestClass* tcReject = new test_utils::TestClass("reject");
      filterTransform(tcReject, resolvedFunc, rejectedFunc);
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
      filterTransform(tcPass, resolvedFunc, rejectedFunc, nextTransform);
      CHECK_FALSE(rejected);
      CHECK(nextPassed);
      CHECK(resolved);
      delete tcPass;
    }

    SUBCASE("Chunk rejected before next transform") {
      test_utils::TestClass* tcReject = new test_utils::TestClass("reject");
      auto nextPassed = false;
      auto nextTransform = [&nextPassed](test_utils::TestClass* chunk,
                                         decltype(resolvedFunc) resolve,
                                         decltype(rejectedFunc)) {
        nextPassed = true;
        resolve(chunk);
      };
      filterTransform(tcReject, resolvedFunc, rejectedFunc, nextTransform);
      CHECK_FALSE(nextPassed);
      CHECK_FALSE(resolved);
      CHECK(rejected);
    }
  }
}

TEST_CASE("FilterTransform no implicit copies") {
  using TestClass = test_utils::TestClass;

  auto filterPassAll = [](TestClass& chunk) -> bool {
    CHECK_EQ(chunk.copy_count_, 0);
    return true;
  };
  auto filterPassNone = [](TestClass& chunk) -> bool {
    CHECK_EQ(chunk.copy_count_, 0);
    return false;
  };

  auto resolvedFunc = [](TestClass& chunk) { CHECK_EQ(chunk.copy_count_, 0); };
  auto rejectedFunc = [] {};
  auto testValue = TestClass();

  auto filterAll = filter(filterPassAll);
  filterAll(testValue, resolvedFunc, rejectedFunc);
  CHECK_EQ(testValue.copy_count_, 0);

  auto filterNone = filter(filterPassNone);
  filterNone(testValue, resolvedFunc, rejectedFunc);
  CHECK_EQ(testValue.copy_count_, 0);
}

}  // namespace ifca