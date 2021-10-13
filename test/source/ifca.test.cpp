#include <doctest/doctest.h>
#include <ifca/version.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <string>

#include "helpers/futureIsReady.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/transform.hpp"
#include "ifca/types.hpp"

namespace ifca {

const std::array<int, 6> kTestSequenceInt = {1, 2, 1, 3, 2, 4};

// TODO: remove after handling all types
decltype(kTestSequenceInt)::const_iterator it_kTestSequenceInt;

std::array<std::string, 6> kTestSequenceString = {
    "napis 1", "napis 2", "napis 3", "napis 4", "napis 5", "napis 6"};
std::array<std::string, 6> kResultSequenceString = {
    "napis 10", "napis 20", "napis 30", "napis 40", "napis 50", "napis 60"};

const unsigned int kMaxParallel = 4;

std::function<chunk_outtype(chunk_intype)> test_string_function =
    [](chunk_intype chunk) { return chunk.append("0"); };

// TODO: remove after handling all types
std::function<chunk_outtype(chunk_intype)> test_delay_function =
    [](chunk_intype chunk) {
      if (it_kTestSequenceInt != kTestSequenceInt.end()) {
        auto delay = *it_kTestSequenceInt++;
        // TODO: change to non blocking thread
        std::this_thread::sleep_for(std::chrono::milliseconds(10 * delay));
      }
      return chunk;
    };

std::function<bool(const chunk_intype &, const chunk_sfuture &)>
    sync_comparator =
        [](const chunk_intype &input, const chunk_sfuture &result) {
          if (!future_is_ready(result)) return false;
          return input == result.get();
        };

const std::array<std::string, 6> kTransformResult = {
    "napis 10", "napis 20", "napis 30", "napis 40", "napis 50", "napis 60"};

// TEST_CASE("write then read concurrently") {
//   auto ifca = Ifca(kMaxParallel);
//   auto t1 = std::unique_ptr<TransformBase>(new
//   Transform(test_string_function)); auto t2 =
//   std::unique_ptr<TransformBase>(new Transform(test_delay_function));
//   t1->SetNextTransform(std::move(t2));
//   ifca.addTransform(std::move(t1));

//   std::vector<chunk_sfuture> results_future;
//   std::vector<std::future<void>> processingTasks;
//   std::vector<chunk_outtype> results;
//   std::mutex results_mutex;

//   auto &&input_size = kTestSequenceString.size();
//   for (auto &&s : kTestSequenceString) {
//     ifca.Write(s);
//     results_future.push_back(std::move(ifca.Read()));
//   }
//   REQUIRE(results_future.size() == input_size);

//   // FIXME: libpthread not attached by cmake for tests
//   // for (auto &&result_future : results_future) {
//   //   auto a = std::async(std::launch::async,
//   //                       [&results_mutex, &results, &result_future] {
//   //                         std::cout << "wchodzi async";

//   //                         result_future.wait();
//   //                         std::lock_guard<std::mutex> m(results_mutex);
//   //                         auto &res = result_future.get();
//   //                         results.push_back(std::move(res));
//   //                       });
//   //   processingTasks.push_back(std::move(a));
//   // }
//   for (auto &&processing : processingTasks) {
//     processing.wait();
//   }

//   auto equal = std::equal(
//       kTestSequenceString.begin(), kTestSequenceString.end(),
//       results.begin(), results.end(), [](chunk_intype &val1, chunk_outtype
//       &val2) -> bool {
//         if (val1.compare(val2) == 0) return true;
//         return false;
//       });
//   REQUIRE(equal == true);
// }

// Basic tests
// -----------

TEST_CASE("Passthrough by default") {
  auto ifca = Ifca(kMaxParallel);

  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  std::vector<std::string> results;
  for (size_t i = 0; i < kTestSequenceString.size(); i++) {
    results.push_back(std::move(ifca.Read().get()));
  }
  for (size_t i = 0; i < results.size(); i++) {
    // should this be CHECK?
    REQUIRE(results[i] == kTestSequenceString[i]);
  }
}

TEST_CASE("Simple transformation") {
  auto ifca = Ifca(kMaxParallel);
  ifca.addTransform(std::move(test_string_function));

  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  std::vector<std::string> results;
  for (size_t i = 0; i < kTestSequenceString.size(); i++) {
    results.push_back(std::move(ifca.Read().get()));
  }
  for (size_t i = 0; i < results.size(); i++) {
    // should this be CHECK?
    REQUIRE(results[i] == kTransformResult[i]);
  }
}

// Ordering tests
// --------------

TEST_CASE("Result order with varying processing time") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  ifca.addTransform(std::move(test_delay_function));

  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  std::vector<std::string> results;
  for (size_t i = 0; i < kTestSequenceString.size(); i++) {
    results.push_back(std::move(ifca.Read().get()));
  }
  REQUIRE(results.size() == kTestSequenceString.size());
  for (size_t i = 0; i < results.size(); i++) {
    // should this be CHECK?
    REQUIRE(results[i] == kTestSequenceString[i]);
  }
}

TEST_CASE("Write and read in turn") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  ifca.addTransform(std::move(test_delay_function));

  std::vector<chunk_sfuture> reads;
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
    reads.push_back(std::move(ifca.Read()));
  }

  std::vector<std::string> results;
  for (auto &&read : reads) {
    auto ready =
        read.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
    CHECK(ready == true);
    if (ready) results.push_back(std::move(read.get()));
  }
  REQUIRE(results.size() == kTestSequenceString.size());
  for (size_t i = 0; i < results.size(); i++) {
    // should this be CHECK?
    REQUIRE(results[i] == kTestSequenceString[i]);
  }
}

TEST_CASE("Reads before write") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  ifca.addTransform(std::move(test_delay_function));
  auto &&input_size = kTestSequenceString.size();

  std::vector<chunk_sfuture> reads;
  for (size_t i = 0; i < input_size; i++) {
    INFO("index: " << i);
    // should this use ifca.Read().share() ? Why?
    reads.push_back(std::move(ifca.Read()));
  }
  for (size_t i = 0; i < input_size; i++) {
    INFO("index: " << i);
    REQUIRE(reads[i].valid() == true);
    CHECK(future_is_ready(reads[i]) == false);
  }

  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }

  std::vector<std::string> results;
  for (auto &&read : reads) {
    results.push_back(std::move(read.get()));
  }
  REQUIRE(results.size() == kTestSequenceString.size());
  for (size_t i = 0; i < results.size(); i++) {
    // should this be CHECK?
    REQUIRE(results[i] == kTestSequenceString[i]);
  }
}

// Filtering tests
// ---------------

// Limits tests
// ------------

TEST_CASE("Writing below limit - no transformation") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<drain_sfuture> drains;
  REQUIRE(kTestSequenceString.size() > kMaxParallel - 1);
  for (size_t i = 0; i < kMaxParallel - 1; i++) {
    auto &s = kTestSequenceString[i];
    drains.push_back(std::move(ifca.Write(s)));
  }

  for (auto &&drain : drains) {
    CHECK(future_is_ready(drain) == true);
  }
}

TEST_CASE("Unrestricted writing below limit (async)") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  ifca.addTransform(std::move(test_delay_function));
  size_t write_count = kMaxParallel - 1;

  std::vector<drain_sfuture> drains;
  for (size_t i = 0; i < write_count; i++) {
    auto &s = kTestSequenceString[i];
    drains.push_back(std::move(ifca.Write(s)));
  }
  for (size_t i = 0; i < write_count; i++) {
    INFO("index: " << i);
    auto &drain = drains[i];
    CHECK(future_is_ready(drain) == true);
  }
}

TEST_CASE("Drain pending when limit reached") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  ifca.addTransform(std::move(test_delay_function));
  REQUIRE(kTestSequenceString.size() > kMaxParallel);

  std::vector<drain_sfuture> drains;
  for (auto &s : kTestSequenceString) {
    drains.push_back(std::move(ifca.Write(s)));
  }
  for (size_t i = 0; i < drains.size(); i++) {
    auto &drain = drains[i];
    INFO("index: " << i);
    if (i < kMaxParallel - 1)
      CHECK(future_is_ready(drain) == true);
    else {
      CHECK(future_is_ready(drain) == false);
    }
  }
}

TEST_CASE("Drain resolved when drops below limit") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  ifca.addTransform(std::move(test_delay_function));
  const auto write_count = kMaxParallel + 2;
  REQUIRE(kTestSequenceString.size() >= write_count);

  std::vector<drain_sfuture> drains;
  for (size_t i = 0; i < write_count; i++) {
    drains.push_back(std::move(ifca.Write(kTestSequenceString[i])));
  }

  for (size_t i = 0; i < write_count; i++) {
    auto &drain = drains[i];
    INFO("For index: " << i);
    if (i < kMaxParallel - 1)
      CHECK(future_is_ready(drain) == true);
    else
      CHECK(future_is_ready(drain) == false);
  }

  std::vector<chunk_outtype> results;
  for (size_t i = 0; i < 3; i++) {
    results.push_back(std::move(ifca.Read().get()));
  }

  for (size_t i = 0; i < write_count; i++) {
    auto &drain = drains[i];
    INFO("For index: " << i);
    CHECK(future_is_ready(drain) == true);
  }
}

// Ending tests
// ------------

// OLLLLLLLLLLLLD

TEST_CASE("reads exceeding writes") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<chunk_sfuture> results;
  auto &&input_size = kTestSequenceString.size();
  auto &extra_reads = kMaxParallel;
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
    results.push_back(std::move(ifca.Read()));
  }
  for (size_t i = 0; i < extra_reads; i++) {
    results.push_back(std::move(ifca.Read()));
  }
  REQUIRE(results.size() == input_size + extra_reads);

  ifca.End();
  auto isMultipleEnd = false;
  try {
    ifca.End();
  } catch (const MultipleEnd &e) {
    isMultipleEnd = true;
  }
  CHECK(isMultipleEnd == true);

  auto exception_count = 0;
  for (size_t i = 0; i < results.size(); i++) {
    auto &result = results[i];
    INFO("index: " << i);
    CHECK(result.valid() == true);
    if (i < input_size) {
      CHECK(future_is_ready(result) == true);
    } else {
      try {
        REQUIRE(future_is_ready(result) == true);
        result.get();
      } catch (const ReadEnd &e) {
        ++exception_count;
      }
    }
  }
  REQUIRE(exception_count == extra_reads);
}

TEST_CASE("reads after end") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<chunk_sfuture> results;
  auto &&input_size = kTestSequenceString.size();
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  ifca.End();
  for (size_t i = 0; i < input_size; i++) {
    results.push_back(std::move(ifca.Read()));
  }
  auto equal =
      std::equal(kTestSequenceString.begin(), kTestSequenceString.end(),
                 results.begin(), sync_comparator);
  REQUIRE(equal == true);
}

}  // namespace ifca