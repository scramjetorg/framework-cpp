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
#include "ifca/ifca.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/types.hpp"
#include "testData.hpp"

namespace ifca {

template <typename InputType, typename OutputType = InputType>
void await_results_concurently(std::vector<InputType> &resultFutures,
                               std::vector<OutputType> &results) {
  std::promise<void> initFinished;
  auto initFinishedFuture = initFinished.get_future();

  std::mutex results_mutex;
  std::vector<std::future<void>> results_futures;

  for (auto &&in : resultFutures) {
    auto future = std::async(
        std::launch::async,
        [&results, &results_mutex, &initFinishedFuture](InputType &&in) {
          initFinishedFuture.wait();
          in.wait();
          std::lock_guard<std::mutex> l(results_mutex);
          results.push_back(in.get());
        },
        std::move(in));
    results_futures.push_back(std::move(future));
  }

  initFinished.set_value();

  for (auto &&result_future : results_futures) {
    result_future.wait();
  }
}

#define TestTypes                                               \
  test_utils::TestData<int>, test_utils::TestData<std::string>, \
      test_utils::TestData<test_utils::TestClass *>,            \
      test_utils::TestData<test_utils::TestClass, std::string>

const unsigned int kMaxParallel = 4;

TEST_CASE_TEMPLATE("IFCA- Basic tests", TestData, TestTypes) {
  using in = typename TestData::input;
  using out = typename TestData::output;
  using emptyIfca = Ifca<in, in>;

  auto testData = TestData();
  auto input = testData.inputSequence();
  auto eachFunc = testData.transformEachFunction();
  auto expectedEachResult = testData.expectedEachResult();

  SUBCASE("Passthrough by default") {
    auto ifca = emptyIfca(kMaxParallel);
    std::vector<in> results;

    for (auto &&s : input) {
      ifca.write(s);
    }
    for (auto i = input.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }

    CHECK(std::equal(input.begin(), input.end(), results.begin()));
  }

  SUBCASE("Simple transformation") {
    auto ifca = emptyIfca(kMaxParallel) + Each(eachFunc);
    std::vector<out> results;

    for (auto &&s : input) {
      ifca.write(s);
    }
    for (auto i = input.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }
    CHECK(
        std::equal(results.begin(), results.end(), expectedEachResult.begin()));
  }

  SUBCASE("Concurrent processing") {
    auto input = test_utils::TestData<int>().inputSequence();
    auto timeResolution = 10000;
    int maxExpectedTime =
        timeResolution * std::accumulate(input.begin(), input.end(),
                                         decltype(input)::value_type(0));
    auto minExpectedTimeIt = std::max_element(input.begin(), input.end());
    int minExpectedTime = timeResolution * (*minExpectedTimeIt);

    std::function<int(int)> delayFunc = [](int chunk) {
      test_utils::Timer::waitFor(std::chrono::milliseconds(chunk * 10));
      return chunk;
    };
    auto ifca = Ifca<int, int>(kMaxParallel) + Each(delayFunc);
    std::vector<int> results;

    for (auto &&s : input) {
      ifca.write(s);
    }
    auto start = std::chrono::system_clock::now();
    for (auto i = input.size(); i--;) {
      results.push_back(std::move(ifca.read().get()));
    }
    auto end = std::chrono::system_clock::now();
    auto time_span =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    CHECK_LE(time_span, maxExpectedTime);
    CHECK_GE(time_span, minExpectedTime);
    CHECK(std::equal(results.begin(), results.end(), input.begin()));
  }
}

TEST_CASE("IFCA- Ordering tests") {
  using in = int;
  using out = in;
  using emptyIfca = Ifca<in, in>;
  auto dataset1 = std::vector<in>{0, 1, 0, 1, 0, 1};
  auto dataset2 = std::vector<in>{1, 3, 2, 6, 4, 5};
  std::function<out(in)> delayFunc = [](in delay) {
    test_utils::Timer::waitFor(std::chrono::milliseconds(delay * 10));
    return delay;
  };

  SUBCASE("Result order with odd chunks delayed") {
    auto ifca = emptyIfca(kMaxParallel) + Each(delayFunc);
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
    auto ifca = emptyIfca(kMaxParallel) + Each(delayFunc);
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
    auto ifca = emptyIfca(kMaxParallel) + Each(delayFunc);
    std::vector<std::future<out>> result_futures;
    decltype(dataset2) results;
    for (auto &&s : dataset2) {
      ifca.write(s);
      result_futures.push_back(std::move(ifca.read()));
    }
    await_results_concurently(result_futures, results);
    CHECK(std::equal(results.begin(), results.end(), dataset2.begin()));
  }

  SUBCASE("Multiple concurrent reads") {
    auto ifca = emptyIfca(kMaxParallel) + Each(delayFunc);
    std::vector<std::future<out>> result_futures;
    decltype(dataset2) results;
    for (auto &&s : dataset2) {
      ifca.write(s);
    }
    for (auto i = dataset2.size(); i--;) {
      result_futures.push_back(std::move(ifca.read()));
    }
    await_results_concurently(result_futures, results);
    CHECK(std::equal(results.begin(), results.end(), dataset2.begin()));
  }

  SUBCASE("Reads before write") {
    auto ifca = emptyIfca(kMaxParallel) + Each(delayFunc);
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
  using emptyIfca = Ifca<in, in>;
  auto dataset1 = std::vector<in>{0, 1, 0, 1, 0, 1};
  auto expectedResults = std::vector<in>(dataset1.size(), 0);
  std::function<bool(in)> filterFunc = [](in chunk) { return !(chunk % 2); };

  SUBCASE("Support for dropping chunks") {
    auto ifca = emptyIfca(kMaxParallel) + Filter(filterFunc);
    decltype(dataset1) results;

    for (auto &&s : dataset1) ifca.write(s);
    for (auto &&s : dataset1) ifca.write(s);
    for (auto i = dataset1.size(); i--;)
      results.push_back(std::move(ifca.read().get()));

    CHECK(std::equal(results.begin(), results.end(), expectedResults.begin()));
  }

  SUBCASE("Reads before filtering") {
    auto ifca = emptyIfca(kMaxParallel) + Filter(filterFunc);
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
    std::function<bool(in)> filterAll = [](in) { return false; };
    bool secondFuncCalled = false;
    std::mutex m;
    std::function<bool(in)> checkCall = [&m,
                                         &secondFuncCalled](in /* chunk */) {
      std::lock_guard<std::mutex> lock(m);
      secondFuncCalled = true;
      return true;
    };
    auto ifca = emptyIfca(kMaxParallel) + Filter(filterAll) + Each(checkCall);
    decltype(dataset1) results;

    for (auto &&s : dataset1) ifca.write(s);
    CHECK_FALSE(secondFuncCalled);
  }
}

TEST_CASE_TEMPLATE("IFCA- Limits tests", TestData, TestTypes) {
  using in = typename TestData::input;
  using out = typename TestData::output;
  using emptyIfca = Ifca<in, in>;

  auto testData = TestData();
  auto input = testData.inputSequence();
  auto eachFunc = testData.transformEachFunction();

  SUBCASE("Unrestricted writing below limit - without transformation") {
    auto ifca = emptyIfca(kMaxParallel);
    std::vector<drain_sfuture> drains;
    REQUIRE_GE(input.size(), kMaxParallel - 1);

    for (size_t i = 0; i < kMaxParallel - 1; i++) {
      auto &&s = input[i];
      drains.push_back(std::move(ifca.write(s)));
    }
    for (auto &&drain : drains) {
      CHECK(future_is_ready(drain));
    }
  }

  SUBCASE("Unrestricted writing below limit") {
    auto ifca = emptyIfca(kMaxParallel) + Each(eachFunc);
    std::vector<drain_sfuture> drains;
    REQUIRE_GE(input.size(), kMaxParallel - 1);

    for (size_t i = 0; i < kMaxParallel - 1; i++) {
      auto &&s = input[i];
      drains.push_back(std::move(ifca.write(s)));
    }
    for (auto &&drain : drains) {
      CHECK(future_is_ready(drain));
    }
  }

  SUBCASE("Drain pending when limit reached") {
    auto ifca = emptyIfca(kMaxParallel) + Each(eachFunc);
    std::vector<drain_sfuture> drains;
    REQUIRE_GE(input.size(), kMaxParallel);

    for (auto &s : input) {
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
    auto ifca = emptyIfca(kMaxParallel) + Each(eachFunc);
    std::vector<drain_sfuture> drains;
    const auto write_count = kMaxParallel + 2;
    REQUIRE_GE(input.size(), write_count);

    for (size_t i = 0; i < write_count; i++) {
      drains.push_back(std::move(ifca.write(input[i])));
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
  using emptyIfca = Ifca<in, in>;

  const unsigned int kMaxParallel = 4;
  auto testData = TestData();
  auto input = testData.inputSequence();

  SUBCASE("Reading from empty Ifca") {
    auto ifca = emptyIfca(kMaxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.read().get(), ReadEnd &);
  }

  SUBCASE("End with pending reads") {
    auto ifca = emptyIfca(kMaxParallel);
    std::vector<std::future<in>> results;
    for (auto i = input.size(); i--;) {
      results.push_back(std::move(ifca.read()));
    }
    ifca.end();

    for (auto &&result : results) {
      CHECK_THROWS_AS(result.get(), ReadEnd &);
    }
  }

  SUBCASE("Write after end errors") {
    auto ifca = emptyIfca(kMaxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.write(input[0]), WriteAfterEnd &);
  }

  SUBCASE("Multiple ends error") {
    auto ifca = emptyIfca(kMaxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.end(), MultipleEnd &);
  }
}

}  // namespace ifca