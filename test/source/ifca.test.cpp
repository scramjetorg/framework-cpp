#include <doctest/doctest.h>
#include <ifca/ifca.h>
#include <ifca/version.h>

#include <string>
#include <iostream>

TEST_CASE("Greeter") {
  using namespace ifca;

  auto cb = [](int a) { return std::to_string(a + 1); };
  IFCA<int, std::string> * ifca = new IFCA<int, std::string>(8, cb);

  ifca -> write(1);

  auto out = ifca -> read().get_future().get();

  if (out.has_value()) {
    auto x = out.value_or("<empty>");
    std::cout << x << std::endl;
  }

  delete ifca;
}

TEST_CASE("Greeter version") {
  static_assert(std::string_view(IFCA_VERSION) == std::string_view("1.0"));
  CHECK(std::string(IFCA_VERSION) == std::string("1.0"));
}
