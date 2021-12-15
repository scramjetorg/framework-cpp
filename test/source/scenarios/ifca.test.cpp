#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <string>

#include "../testClass.hpp"
#include "helpers/futureIsReady.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/types.hpp"
#include "testData.hpp"

namespace ifca {

// const std::array<int, 6> kTestSequenceInt = {1, 2, 1, 3, 2, 4};

// // TODO: remove after handling all types
// decltype(kTestSequenceInt)::const_iterator it_kTestSequenceInt;

std::vector<std::string> kTestSequenceString{"napis 1", "napis 2", "napis3",
                                             "napis 4", "napis 5", "napis 6 "};

// const std::vector<chunk_outtype> kTransformResult{
//     "napis 10", "napis 20", "napis 30", "napis 40", "napis 50", "napis
//     60"};

const unsigned int kMaxParallel = 4;

// std::function<chunk_outtype(chunk_intype)> test_string_function =
//     [](chunk_intype chunk) { return chunk.append("0"); };

// // TODO: remove after handling all types
// std::function<chunk_outtype(chunk_intype)> test_delay_function =
//     [](chunk_intype chunk) {
//       if (it_kTestSequenceInt != kTestSequenceInt.end()) {
//         auto delay = *it_kTestSequenceInt++;
//         // TODO: change to non blocking thread
//         std::this_thread::sleep_for(std::chrono::milliseconds(10 * delay));
//       }
//       return chunk;
//     };

// bool in_out_comparator(chunk_intype &val1, chunk_outtype &val2) {
//   if (val1.compare(val2) == 0) return true;
//   return false;
// }

// bool in_sfuture_comparator(const chunk_intype &input,
//                            const chunk_sfuture &result) {
//   if (!future_is_ready(result)) return false;
//   return input == result.get();
// }

// bool isEqual(std::vector<chunk_intype> input,
//              std::vector<chunk_outtype> output) {
//   return std::equal(input.begin(), input.end(), output.begin(),
//                     in_out_comparator);
// }

// bool isEqual(std::vector<chunk_intype> input,
//              std::vector<chunk_sfuture> output) {
//   return std::equal(input.begin(), input.end(), output.begin(),
//                     in_sfuture_comparator);
// }

// void await_results_concurently(std::vector<chunk_sfuture> &input,
//                                std::vector<chunk_outtype> &results) {
//   std::mutex results_mutex;
//   std::vector<std::future<void>> results_futures;
//   for (auto &&in : input) {
//     auto future = std::async(
//         std::launch::async,
//         [&results, &results_mutex](chunk_sfuture &&in) {
//           in.wait();
//           std::lock_guard<std::mutex> l(results_mutex);
//           results.push_back(in.get());
//         },
//         std::move(in));
//     results_futures.push_back(std::move(future));
//   }
//   for (auto &&result_futures : results_futures) {
//     result_futures.wait();
//   }
// }

// Basic tests
// -----------

#define TestTypes                                               \
  test_utils::TestData<int>, test_utils::TestData<std::string>, \
      test_utils::TestData<test_utils::TestClass *>,            \
      test_utils::TestData<test_utils::TestClass, std::string>

TEST_CASE_TEMPLATE("IFCA", TestData, TestTypes) {
  using in = typename TestData::input;
  using out = typename TestData::output;
  using emptyIfca = Ifca<in, in>;

  const unsigned int maxParallel = 4;
  auto testData = TestData();
  auto input = testData.inputSequence();
  auto eachFunc = testData.transformEachFunction();
  auto expectedEachResult = testData.expectedEachResult();

  SUBCASE("Passthrough by default") {
    auto ifca = emptyIfca(maxParallel);
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
    auto ifca = emptyIfca(maxParallel) + Each(eachFunc);
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
    // auto ifca = emptyIfca(maxParallel) + Each(eachFunc);
    // std::vector<out> results;

    // for (auto &&s : input) {
    //   ifca.write(s);
    // }
    // for (size_t i = 0; i < input.size(); i++) {
    //   results.push_back(std::move(ifca.read()));
    // }

    //   std::vector<std::string> results;
    //   auto t1 = std::chrono::high_resolution_clock::now();
    //   await_results_concurently(reads, results);
    //   auto t2 = std::chrono::high_resolution_clock::now();
    //   std::chrono::duration<double> time_span =
    //       std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    //   auto max_delay_el =
    //       std::max_element(kTestSequenceInt.begin(), kTestSequenceInt.end());
    //   auto max_delay = *max_delay_el * 0.2;
    //   CHECK_LE(time_span.count(), max_delay);
    //   CHECK(isEqual(results, kTestSequenceString));
  }

  // // Ordering tests
  // // --------------

  // TEST_CASE("Result order with odd chunks delayed") {
  //   // TODO: add after generic transforms
  // }

  // TEST_CASE("Result order with varying processing time") {
  //   auto ifca = Ifca(kMaxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);
  //   std::vector<std::string> results;

  //   for (auto &&s : kTestSequenceString) {
  //     ifca.Write(s);
  //   }
  //   for (size_t i = 0; i < kTestSequenceString.size(); i++) {
  //     results.push_back(std::move(ifca.Read().get()));
  //   }

  //   CHECK(isEqual(results, kTestSequenceString));
  // }

  // TEST_CASE("Write and read in turn") {
  //   auto ifca = Ifca(kMaxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);

  //   std::vector<chunk_sfuture> reads;
  //   for (auto &&s : kTestSequenceString) {
  //     ifca.Write(s);
  //     reads.push_back(std::move(ifca.Read()));
  //   }
  //   std::vector<std::string> results;
  //   await_results_concurently(reads, results);
  //   CHECK(isEqual(results, kTestSequenceString));
  // }

  // TEST_CASE("Multiple concurrent reads") {
  //   auto ifca = Ifca(kMaxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);

  //   std::vector<chunk_sfuture> reads;
  //   for (auto &&s : kTestSequenceString) {
  //     ifca.Write(s);
  //   }
  //   for (size_t i = 0; i < kTestSequenceString.size(); i++) {
  //     reads.push_back(std::move(ifca.Read()));
  //   }
  //   std::vector<std::string> results;
  //   await_results_concurently(reads, results);
  //   CHECK(isEqual(results, kTestSequenceString));
  // }

  // TEST_CASE("Reads before write") {
  //   auto ifca = Ifca(kMaxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);
  //   auto &&input_size = kTestSequenceString.size();

  //   std::vector<chunk_sfuture> reads;
  //   for (size_t i = 0; i < input_size; i++) {
  //     INFO("index: " << i);
  //     reads.push_back(std::move(ifca.Read()));
  //   }
  //   for (size_t i = 0; i < input_size; i++) {
  //     INFO("index: " << i);
  //     REQUIRE(reads[i].valid());
  //     CHECK_FALSE(future_is_ready(reads[i]));
  //   }

  //   for (auto &&s : kTestSequenceString) {
  //     ifca.Write(s);
  //   }
  //   std::vector<chunk_outtype> results;
  //   for (auto &&read : reads) {
  //     results.push_back(std::move(read.get()));
  //   }
  //   CHECK(isEqual(results, kTestSequenceString));
  // }

  // // Filtering tests
  // // ---------------

  // TEST_CASE("Support for dropping chunks") {
  //   // TODO: Add after finished transforms
  // }
  // TEST_CASE("Reads before filtering") {
  //   // TODO: Add after finished transforms
  // }
  // TEST_CASE("Dropping chunks in the middle of chain") {
  //   // TODO: Add after finished transforms
  // }
  // TEST_CASE("Garbage collection of dropped chunks") {
  //   // TODO: Add after finished transforms
  // }

  // // Limits tests
  // // ------------

  // TEST_CASE("Unrestricted writing below limit - without transformation") {
  //   auto ifca = Ifca(kMaxParallel);
  //   std::vector<drain_sfuture> drains;
  //   REQUIRE_GE(kTestSequenceString.size(), kMaxParallel - 1);
  //   for (size_t i = 0; i < kMaxParallel - 1; i++) {
  //     auto &s = kTestSequenceString[i];
  //     drains.push_back(std::move(ifca.Write(s)));
  //   }

  //   for (auto &&drain : drains) {
  //     CHECK(future_is_ready(drain));
  //   }
  // }

  // TEST_CASE("Unrestricted writing below limit") {
  //   auto ifca = Ifca(kMaxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);
  //   size_t write_count = kMaxParallel - 1;
  //   REQUIRE_GE(kTestSequenceString.size(), write_count);

  //   std::vector<drain_sfuture> drains;
  //   for (size_t i = 0; i < write_count; i++) {
  //     auto &s = kTestSequenceString[i];
  //     drains.push_back(std::move(ifca.Write(s)));
  //   }
  //   for (auto &&drain : drains) {
  //     CHECK(future_is_ready(drain));
  //   }
  // }

  // TEST_CASE("Drain pending when limit reached") {
  //   auto ifca = Ifca(kMaxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);
  //   REQUIRE_GE(kTestSequenceString.size(), kMaxParallel);

  //   std::vector<drain_sfuture> drains;
  //   for (auto &s : kTestSequenceString) {
  //     drains.push_back(std::move(ifca.Write(s)));
  //   }
  //   for (size_t i = 0; i < drains.size(); i++) {
  //     auto &drain = drains[i];
  //     INFO("index: " << i);
  //     if (i < kMaxParallel - 1)
  //       CHECK(future_is_ready(drain));
  //     else {
  //       CHECK_FALSE(future_is_ready(drain));
  //     }
  //   }
  // }

  // SUBCASE("Drain resolved when drops below limit") {
  //   auto ifca = emptyIfca(maxParallel);
  //   it_kTestSequenceInt = kTestSequenceInt.begin();
  //   ifca.addTransform(test_delay_function);
  //   const auto write_count = kMaxParallel + 2;
  //   REQUIRE_GE(kTestSequenceString.size(), write_count);

  //   std::vector<drain_sfuture> drains;
  //   for (size_t i = 0; i < write_count; i++) {
  //     drains.push_back(std::move(ifca.Write(kTestSequenceString[i])));
  //   }

  //   std::vector<chunk_outtype> results;
  //   for (size_t i = 0; i < 3; i++) {
  //     results.push_back(std::move(ifca.Read().get()));
  //   }

  //   for (size_t i = 0; i < write_count; i++) {
  //     auto &drain = drains[i];
  //     INFO("For index: " << i);
  //     CHECK(future_is_ready(drain));
  //   }
  // }

  // Ending tests
  // ------------

  SUBCASE("Reading from empty Ifca") {
    auto ifca = emptyIfca(maxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.read().get(), ReadEnd &);
  }

  SUBCASE("End with pending reads") {
    auto ifca = emptyIfca(maxParallel);
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
    auto ifca = emptyIfca(maxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.write(input[0]), WriteAfterEnd &);
  }

  SUBCASE("Multiple ends error") {
    auto ifca = emptyIfca(maxParallel);
    ifca.end();
    CHECK_THROWS_AS(ifca.end(), MultipleEnd &);
  }
}

}  // namespace ifca