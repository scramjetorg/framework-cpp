#include "ifca/ifca.hpp"

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
        std::this_thread::sleep_for(std::chrono::seconds(delay));
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

TEST_CASE("write then read sequentially") {
  auto ifca = Ifca(kMaxParallel);
  it_kTestSequenceInt = kTestSequenceInt.begin();
  auto t1 = std::unique_ptr<TransformBase>(new Transform(test_string_function));
  auto t2 = std::unique_ptr<TransformBase>(new Transform(test_delay_function));
  t1->SetNextTransform(std::move(t2));
  ifca.addTransform(std::move(t1));

  std::vector<chunk_outtype> results;
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  for (size_t i = 0; i < kTestSequenceString.size(); i++) {
    results.push_back(std::move(ifca.Read().get()));
  }

  REQUIRE(results.size() == kTestSequenceString.size());
  REQUIRE(results.size() == kResultSequenceString.size());
  for (size_t i = 0; i < results.size(); i++) {
    CHECK(results[i] == kResultSequenceString[i]);
  }
}

TEST_CASE("write and read in turn") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<chunk_sfuture> results;
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
    results.push_back(std::move(ifca.Read().share()));
  }

  auto equal =
      std::equal(kTestSequenceString.begin(), kTestSequenceString.end(),
                 results.begin(), sync_comparator);
  REQUIRE(equal == true);
}

TEST_CASE("reads before write") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<chunk_sfuture> results;
  auto &&input_size = kTestSequenceString.size();
  for (size_t i = 0; i < input_size; i++) {
    INFO("index: " << i);
    results.push_back(std::move(ifca.Read().share()));
  }

  for (size_t i = 0; i < input_size; i++) {
    INFO("index: " << i);
    REQUIRE(results[i].valid() == true);
    CHECK(future_is_ready(results[i]) == false);
  }
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  for (size_t i = 0; i < input_size; i++) {
    INFO("index: " << i);
    REQUIRE(results[i].valid() == true);
    CHECK(future_is_ready(results[i]) == true);
  }

  auto equal =
      std::equal(kTestSequenceString.begin(), kTestSequenceString.end(),
                 results.begin(), sync_comparator);
  REQUIRE(equal == true);
}

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

TEST_CASE("synchronous draining") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<drain_sfuture> drains;
  for (size_t i = 0; i < kMaxParallel; i++) {
    auto &s = kTestSequenceString[i];
    drains.push_back(std::move(ifca.Write(s)));
  }
  for (auto &&drain : drains) {
    CHECK(future_is_ready(drain) == true);
  }
}

TEST_CASE("drain waiting for reads") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<drain_sfuture> drains;
  for (auto &s : kTestSequenceString) {
    drains.push_back(std::move(ifca.Write(s)));
  }
  for (size_t i = 0; i < drains.size(); i++) {
    auto &drain = drains[i];
    INFO("index: " << i);
    if (i < kMaxParallel)
      CHECK(future_is_ready(drain) == true);
    else {
      CHECK(future_is_ready(drain) == false);
    }
  }
  for (size_t i = 0; i < drains.size() - kMaxParallel; i++) {
    ifca.Read();
  }

  for (size_t i = 0; i < drains.size(); i++) {
    auto &drain = drains[i];
    REQUIRE(drain.valid() == true);
    INFO("For index: " << i);
    CHECK(future_is_ready(drain) == true);
  }
}

TEST_CASE("writing above limit") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<drain_sfuture> drains;
  for (auto &s : kTestSequenceString) {
    drains.push_back(std::move(ifca.Write(s)));
  }
  for (size_t i = 0; i < drains.size(); i++) {
    auto &drain = drains[i];
    INFO("index: " << i);
    if (i < kMaxParallel)
      CHECK(future_is_ready(drain) == true);
    else {
      CHECK(future_is_ready(drain) == false);
    }
  }
}

TEST_CASE("empty transformation chain") {
  auto ifca = Ifca(kMaxParallel);
  std::vector<chunk_sfuture> results;
  auto &&input_size = kTestSequenceString.size();
  for (auto &&s : kTestSequenceString) {
    ifca.Write(s);
  }
  for (size_t i = 0; i < input_size; i++) {
    results.push_back(std::move(ifca.Read()));
  }

  auto equal =
      std::equal(kTestSequenceString.begin(), kTestSequenceString.end(),
                 results.begin(), sync_comparator);
  REQUIRE(equal == true);
}

}  // namespace ifca