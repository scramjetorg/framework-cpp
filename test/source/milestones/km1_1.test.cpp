#include <doctest/doctest.h>

#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/each.hpp"

namespace ifca {

const unsigned int MAX_PARALLEL = 4;

struct TestClass {
  TestClass(int idx) : sampleTxt("TestClass"), index(idx), copies(0){};
  TestClass(const TestClass&) { ++copies; };
  TestClass& operator=(const TestClass&) {
    ++copies;
    return *this;
  }
  bool operator==(const TestClass& t) const {
    return (sampleTxt == t.sampleTxt) && (index == t.index);
  }

  friend std::ostream& operator<<(std::ostream& os, const TestClass& tc) {
    os << tc.sampleTxt << " index: " << std::to_string(tc.index);
    return os;
  }

  std::string sampleTxt;
  int index;
  unsigned int copies;
};

TEST_CASE("Ifca") {
  // Create unmutating data test functions
  std::function<TestClass&(TestClass&)> passUnchanged1 =
      [](TestClass& t) -> TestClass& {
    LOG_INFO() << "passUnchanged1 for: " << t;
    return t;
  };
  std::function<TestClass&(TestClass&)> passUnchanged2 =
      [](TestClass& t) -> TestClass& {
    LOG_INFO() << "passUnchanged2 for: " << t;
    return t;
  };
  std::function<TestClass&(TestClass&)> passUnchanged3 =
      [](TestClass& t) -> TestClass& {
    LOG_INFO() << "passUnchanged3 for: " << t;
    return t;
  };

  std::function<TestClass&(TestClass&)> passUnchanged4 =
      [](TestClass& t) -> TestClass& {
    LOG_INFO() << "passUnchanged4 for: " << t;
    return t;
  };

  // Create test data
  TestClass data0(0);
  TestClass data1(1);
  TestClass data2(2);
  TestClass data3(3);
  TestClass data4(4);
  TestClass data5(5);
  TestClass data6(6);

  // Create ifca
  auto ifca = Ifca<TestClass&>(MAX_PARALLEL)
                  .addTransform(each(passUnchanged1))
                  .addTransform(each(passUnchanged2))
                  .addTransform(each(passUnchanged3))
                  .addTransform(each(passUnchanged4));

  // Write test data to ifca
  ifca.write(data0);
  ifca.write(data1);
  ifca.write(data2);
  ifca.write(data3);
  ifca.write(data4);
  ifca.write(data5);
  ifca.write(data6);

  // Read future results
  auto futureResult0 = ifca.read();
  auto futureResult1 = ifca.read();
  auto futureResult2 = ifca.read();
  auto futureResult3 = ifca.read();
  auto futureResult4 = ifca.read();
  auto futureResult5 = ifca.read();
  auto futureResult6 = ifca.read();

  // Wait for futures values
  auto& result0 = futureResult0.get();
  auto& result1 = futureResult1.get();
  auto& result2 = futureResult2.get();
  auto& result3 = futureResult3.get();
  auto& result4 = futureResult4.get();
  auto& result5 = futureResult5.get();
  auto& result6 = futureResult6.get();

  // Check if data is not implicitly copied
  CHECK_EQ(&data0, &result0);
  CHECK_EQ(data0.copies, 0);
  CHECK_EQ(&data1, &result1);
  CHECK_EQ(data1.copies, 0);
  CHECK_EQ(&data2, &result2);
  CHECK_EQ(data2.copies, 0);
  CHECK_EQ(&data3, &result3);
  CHECK_EQ(data3.copies, 0);
  CHECK_EQ(&data4, &result4);
  CHECK_EQ(data4.copies, 0);
  CHECK_EQ(&data5, &result5);
  CHECK_EQ(data5.copies, 0);
  CHECK_EQ(&data6, &result6);
  CHECK_EQ(data6.copies, 0);

  // Check if data is not changed
  CHECK_EQ(data0, result0);
  CHECK_EQ(data1, result1);
  CHECK_EQ(data2, result2);
  CHECK_EQ(data3, result3);
  CHECK_EQ(data4, result4);
  CHECK_EQ(data5, result5);
  CHECK_EQ(data6, result6);
}

}  // namespace ifca