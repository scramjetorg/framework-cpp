#include <doctest/doctest.h>

#include "helpers/Logger/logger.hpp"
#include "ifca/drain.hpp"

TEST_CASE("Drain") {
  const auto maxParallel = 4;
  auto state = ifca::DrainState(maxParallel);

  SUBCASE("Drain state: occured and drained") {
    CHECK(state.drained());

    for (auto i = maxParallel; i--;) state.ChunkStartedProcessing();
    CHECK(state.drainOccured());

    state.ChunkFinishedProcessing();
    CHECK(state.drainOccured());

    state.ChunkRead();
    CHECK(state.drained());
  }

  SUBCASE("Drain state: occured and drained without processing") {
    CHECK(state.drained());

    for (auto i = maxParallel; i--;) state.ChunkReadReady();
    CHECK(state.drainOccured());

    state.ChunkRead();
    CHECK(state.drained());
  }
}