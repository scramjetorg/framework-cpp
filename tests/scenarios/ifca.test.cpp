#include "ifca/ifca.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <numeric>

#include "../testClass.hpp"
#include "helpers/futureIsReady.hpp"
#include "helpers/timer.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/types.hpp"
#include "testData.hpp"

namespace ifca {

#define TestTypes                                               \
  test_utils::TestData<int>, test_utils::TestData<std::string>, \
      test_utils::TestData<test_utils::TestClass *>,            \
      test_utils::TestData<test_utils::TestClass, std::string>

const unsigned int kMaxParallel = 4;

TEST_CASE_TEMPLATE("IFCA- Basic tests", TestData, TestTypes) {
  using in = typename TestData::input;
  using out = typename TestData::output;

  auto data = TestData();
  auto testSequence = data.testSequence();
  auto modifyData = data.modifyData();
  auto modifiedTestSequence = data.modifiedTestSequence();

  SUBCASE("Passthrough by default") {
    auto ifca = Ifca<in>(kMaxParallel);
    std::vector<in> results;

    for (auto &&s : testSequence) {
      ifca.write(s);
    }
    for (auto i = testSequence.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }

    CHECK(
        std::equal(testSequence.begin(), testSequence.end(), results.begin()));
  }

  SUBCASE("Simple transformation") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(modifyData));
    std::vector<out> results;

    for (auto &&s : testSequence) {
      ifca.write(s);
    }
    for (auto i = testSequence.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }
    CHECK(std::equal(results.begin(), results.end(),
                     modifiedTestSequence.begin()));
  }

  SUBCASE("Concurrent processing") {
    auto testSequenceInt = test_utils::TestData<int>().testSequence();
    auto timeResolution = 10000;
    int maxExpectedTime =
        timeResolution *
        std::accumulate(testSequenceInt.begin(), testSequenceInt.end(),
                        decltype(testSequenceInt)::value_type(0));
    auto minExpectedTimeIt =
        std::max_element(testSequenceInt.begin(), testSequenceInt.end());
    int minExpectedTime = timeResolution * (*minExpectedTimeIt);

    auto delayFunc = [](int chunk) {
      test_utils::Timer::waitFor(std::chrono::milliseconds(chunk * 10));
      return chunk;
    };
    auto ifca = Ifca(kMaxParallel).addTransform(each(delayFunc));
    std::vector<int> results;

    for (auto &&s : testSequenceInt) {
      ifca.write(s);
    }
    auto start = std::chrono::system_clock::now();
    for (auto i = testSequenceInt.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }
    auto end = std::chrono::system_clock::now();
    auto time_span =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    CHECK_LE(time_span, maxExpectedTime);
    CHECK_GE(time_span, minExpectedTime);
    CHECK(std::equal(results.begin(), results.end(), testSequenceInt.begin()));
  }
}

TEST_CASE("IFCA- Ordering tests") {
  using in = int;
  using out = in;
  auto dataset1 = std::vector<in>{0, 1, 0, 1, 0, 1};
  auto dataset2 = std::vector<in>{1, 3, 2, 6, 4, 5};
  auto delayFunc = [](in delay) {
    test_utils::Timer::waitFor(std::chrono::milliseconds(delay * 10));
    return delay;
  };

  SUBCASE("Result order with odd chunks delayed") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(delayFunc));
    decltype(dataset1) results;
    for (auto &&s : dataset1) {
      ifca.write(s);
    }
    for (auto i = dataset1.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }
    CHECK(std::equal(results.begin(), results.end(), dataset1.begin()));
  }

  SUBCASE("Result order with varying processing time") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(delayFunc));
    decltype(dataset2) results;
    for (auto &&s : dataset2) {
      ifca.write(s);
    }
    for (auto i = dataset2.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }
    CHECK(std::equal(results.begin(), results.end(), dataset2.begin()));
  }

  SUBCASE("Write and read in turn") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(delayFunc));
    std::vector<std::future<out>> result_futures;
    decltype(dataset2) results;
    for (auto &&s : dataset2) {
      ifca.write(s);
      result_futures.push_back(std::move(ifca.read()));
    }
    for (auto &&res : result_futures) {
      results.push_back(res.get());
    }
    CHECK(std::equal(results.begin(), results.end(), dataset2.begin()));
  }

  SUBCASE("Multiple concurrent reads") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(delayFunc));
    std::vector<std::future<out>> result_futures;
    decltype(dataset2) results;
    for (auto &&s : dataset2) {
      ifca.write(s);
    }
    for (auto i = dataset2.size(); i--;) {
      result_futures.push_back(std::move(ifca.read()));
    }
    for (auto &&res : result_futures) {
      results.push_back(res.get());
    }
    CHECK(std::equal(results.begin(), results.end(), dataset2.begin()));
  }

  SUBCASE("Reads before write") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(delayFunc));
    std::vector<std::future<out>> result_futures;
    decltype(dataset2) results;
    for (auto i = dataset2.size(); i--;) {
      result_futures.push_back(std::move(ifca.read()));
    }
    for (auto i = result_futures.size(); i--;) {
      CHECK_FALSE(future_is_ready(result_futures[i]));
    }
    for (auto &&s : dataset2) {
      ifca.write(s);
    }
    for (auto &&read : result_futures) {
      results.push_back(std::move(read.get()));
    }
    CHECK(std::equal(results.begin(), results.end(), dataset2.begin()));
  }
}

TEST_CASE("IFCA- Filtering tests") {
  using in = int;
  auto dataset1 = std::vector<in>{0, 1, 0, 1, 0, 1};
  auto expectedResults = std::vector<in>(dataset1.size(), 0);
  auto filterFunc = [](in chunk) { return !(chunk % 2); };

  SUBCASE("Support for dropping chunks") {
    auto ifca = Ifca(kMaxParallel).addTransform(filter(filterFunc));
    decltype(dataset1) results;

    for (auto &&s : dataset1) ifca.write(s);
    for (auto &&s : dataset1) ifca.write(s);
    for (auto i = dataset1.size(); i--;)
      results.push_back(std::move(ifca.read().get()));

    CHECK(std::equal(results.begin(), results.end(), expectedResults.begin()));
  }

  SUBCASE("Reads before filtering") {
    auto ifca = Ifca(kMaxParallel).addTransform(filter(filterFunc));
    decltype(dataset1) results;
    std::vector<std::future<in>> result_futures;

    for (auto i = dataset1.size(); i--;)
      result_futures.push_back(std::move(ifca.read()));
    for (auto &&s : dataset1) ifca.write(s);
    for (auto &&s : dataset1) ifca.write(s);
    for (auto i = result_futures.size(); i--;)
      results.push_back(std::move(result_futures[i].get()));

    CHECK(std::equal(results.begin(), results.end(), expectedResults.begin()));
  }

  SUBCASE("Dropping chunks in the middle of chain") {
    auto filterAll = [](in) { return false; };
    bool secondFuncCalled = false;
    std::mutex m;
    auto checkCall = [&m, &secondFuncCalled](in) {
      std::lock_guard<std::mutex> lock(m);
      secondFuncCalled = true;
      return true;
    };
    auto ifca = Ifca(kMaxParallel)
                    .addTransform(filter(filterAll))
                    .addTransform(each(checkCall));
    decltype(dataset1) results;

    for (auto &&s : dataset1) ifca.write(s);
    CHECK_FALSE(secondFuncCalled);
  }
}

TEST_CASE_TEMPLATE("IFCA- Limits tests", TestData, TestTypes) {
  using in = typename TestData::input;
  using out = typename TestData::output;

  auto data = TestData();
  auto testSequence = data.testSequence();
  auto modifyData = data.modifyData();

  SUBCASE("Unrestricted writing below limit - without transformation") {
    auto ifca = Ifca<in>(kMaxParallel);
    std::vector<drain_sfuture> drains;
    REQUIRE_GE(testSequence.size(), kMaxParallel - 1);

    for (size_t i = 0; i < kMaxParallel - 1; i++) {
      auto &&s = testSequence[i];
      drains.push_back(std::move(ifca.write(s)));
    }
    for (auto &&drain : drains) {
      CHECK(future_is_ready(drain));
    }
  }

  SUBCASE("Unrestricted writing below limit") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(modifyData));
    std::vector<drain_sfuture> drains;
    REQUIRE_GE(testSequence.size(), kMaxParallel - 1);

    for (size_t i = 0; i < kMaxParallel - 1; i++) {
      auto &&s = testSequence[i];
      drains.push_back(std::move(ifca.write(s)));
    }
    for (auto &&drain : drains) {
      CHECK(future_is_ready(drain));
    }
  }

  SUBCASE("Drain pending when limit reached") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(modifyData));
    std::vector<drain_sfuture> drains;
    REQUIRE_GE(testSequence.size(), kMaxParallel);

    for (auto &s : testSequence) {
      drains.push_back(std::move(ifca.write(s)));
    }
    for (size_t i = 0; i < drains.size(); i++) {
      auto &drain = drains[i];
      INFO("index: " << i);
      if (i < kMaxParallel - 1)
        CHECK(future_is_ready(drain));
      else {
        CHECK_FALSE(future_is_ready(drain));
      }
    }
  }

  SUBCASE("Drain resolved when drops below limit") {
    auto ifca = Ifca(kMaxParallel).addTransform(each(modifyData));
    std::vector<drain_sfuture> drains;
    const auto write_count = kMaxParallel + 2;
    REQUIRE_GE(testSequence.size(), write_count);

    for (size_t i = 0; i < write_count; i++) {
      drains.push_back(std::move(ifca.write(testSequence[i])));
    }

    for (size_t i = kMaxParallel - 1; i < write_count; i++) {
      auto &drain = drains[i];
      INFO("For index: " << i);
      CHECK_FALSE(future_is_ready(drain));
    }

    std::vector<out> results;
    for (size_t i = 0; i < 3; i++) {
      results.push_back(std::move(ifca.read().get()));
    }

    for (size_t i = 0; i < write_count; i++) {
      auto &drain = drains[i];
      INFO("For index: " << i);
      CHECK(future_is_ready(drain));
    }
  }
}

TEST_CASE_TEMPLATE("IFCA- Ending tests", TestData, TestTypes) {
  using in = typename TestData::input;

  auto data = TestData();
  auto testSequence = data.testSequence();

  SUBCASE("Reading from empty Ifca") {
    auto ifca = Ifca<in>(kMaxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.read().get(), ReadEnd &);
  }

  SUBCASE("End with pending reads") {
    auto ifca = Ifca<in>(kMaxParallel);
    std::vector<std::future<in>> results;
    for (auto i = testSequence.size(); i--;) {
      results.push_back(std::move(ifca.read()));
    }
    ifca.end();

    for (auto &&result : results) {
      CHECK_THROWS_AS(result.get(), ReadEnd &);
    }
  }

  SUBCASE("Write after end errors") {
    auto ifca = Ifca<in>(kMaxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.write(testSequence[0]), WriteAfterEnd &);
  }

  SUBCASE("Multiple ends error") {
    auto ifca = Ifca<in>(kMaxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.end(), MultipleEnd &);
  }
}

}  // namespace ifca