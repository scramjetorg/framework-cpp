#include "ifca/transform/delegate.hpp"

#include <doctest/doctest.h>

#include <functional>
#include <string>

#include "../../testClass.hpp"
#include "helpers/Logger/logger.hpp"

namespace detail {

int freeFunc(std::string) { return 0; }

void curriedFunc(char, int) {}

struct TestClass {
  void MemberFunction(int) {}
};

TEST_CASE("Delegate") {
  delegate<int(std::string)> myDel;
  myDel.connect<&freeFunc>();
  LOG_DEBUG() << myDel("test");

  delegate<void(int)> myDelegate;
  TestClass instance;
  myDelegate.connect<&TestClass::MemberFunction>(&instance);
  myDelegate(11);

  myDelegate.connect<&curriedFunc>('c');
  myDelegate(11);

  // delegate my_delegate(&freeFunc);
  // my_delegate("test");

  //   std::function<int(std::string)> function = [](std::string) { return 0; };
  //   auto&& lambda = [](std::string) { return 0; };

  //   detail::Delegate<int(std::string)> functionWrapped(function);
  //   detail::Delegate<int(std::string)> lambdaWrapped(lambda);

  //   CHECK_EQ(functionWrapped(), 0);
  //   CHECK_EQ(lambdaWrapped(), 0);

  // std::function<int(std::string)> stdWrapper;
  // stdWrapper = freeFunc;
  // CHECK_EQ(stdWrapper(""), 0);
  // stdWrapper = [](std::string) { return 0; };
  // CHECK_EQ(stdWrapper(""), 0);
  // CHECK(std::is_same_v<decltype(stdWrapper),
  // std::function<int(std::string)>>);
}

}  // namespace detail