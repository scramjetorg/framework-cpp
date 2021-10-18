#include <doctest/doctest.h>

// #include <functional>
#include <iostream>
#include <iterator>
#include <string>
// #include <type_traits>
// #include <typeinfo>

#include "helpers/FWD.hpp"
#include "helpers/crtp.hpp"
#include "ifca/transform/outStream.hpp"
#include "ifca/transform/each.hpp"
#include "ifca/transform/filter.hpp"
#include "ifca/transform/into.hpp"
#include "ifca/transform/transformChain.hpp"
#include "ifca/transform/transformExpression.hpp"

namespace ifca {

TEST_CASE("Transform") {
  std::function f1 = [](int chunk) {
    std::cout << "function1 call " << chunk << "\n";
    return chunk * 2;
  };
  std::function f2 = [](int chunk) {
    std::cout << "function2 call " << chunk << "\n";
    return chunk + 2;
  };
  std::function f3 = [](int chunk) {
    std::cout << "function3 call " << chunk << "\n";
    return chunk - 2;
  };
  std::function fil1 = [](int chunk) {
    std::cout << "filter call " << chunk << "\n";
    return chunk % 2;
  };
  auto&& filter = Filter(fil1);
  auto&& t1 = Each(f1);
  auto&& t2 = Each(f2);
  auto&& t3 = Each(f3);
  auto&& t4 = OutStream(std::cout);

  // auto c2 = filter |= t1 |= t2 |= t3 |= t4;
  auto c2 = filter >>= t1 >>= t2 >>= t3 >>= t4;
  c2(10);
  c2(11);

  auto c3 = Filter(fil1) >>= Each(f1) >>= OutStream(std::cout);
  c3(21);
  c3(20);
}

}  // namespace ifca